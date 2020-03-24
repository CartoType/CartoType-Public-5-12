/*
cartotype_geometry.h
Copyright (C) CartoType Ltd 2019.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_GEOMETRY__
#define CARTOTYPE_GEOMETRY__

#include <cartotype_path.h>
#include <cartotype_map_object.h>

namespace CartoType
{

/** A template class to hold geometry objects containing various types of point. The point type must be derived from or trivially convertible to TOutlinePointFP. */
template<class point_t> class CGeneralGeometry
    {
    public:
    CGeneralGeometry() { }
    /** Creates a geometry object with given coordinates and open/closed status. */
    explicit CGeneralGeometry(TCoordType aCoordType,bool aClosed = false):
        m_coord_type(aCoordType),
        m_closed(aClosed)
        {
        }
    /**  Creates a geometry object by copying a path, using given coordinates and open/closed status. Ignores the open/closed status of individual contours in the path. */
    CGeneralGeometry(const MPath& aPath,TCoordType aCoordType,bool aClosed):
        m_coord_type(aCoordType),
        m_closed(aClosed)
        {
        for (size_t i = 0; i < aPath.Contours(); i++)
            {
            TContour contour = aPath.Contour(i);
            BeginContour();
            auto& c = m_contour_array.back();
            for (const auto& p : contour)
                c.push_back(point_t(p));
            }
        }
    /** Creates a closed geometry object from an axis-aligned rectangle. */
    CGeneralGeometry(const TRectFP& aRect,TCoordType aCoordType):
        m_coord_type(aCoordType),
        m_closed(true)
        {
        AppendPoint(aRect.iTopLeft);
        AppendPoint(aRect.BottomLeft());
        AppendPoint(aRect.iBottomRight);
        AppendPoint(aRect.TopRight());
        }
    /** Creates an open geometry object containing a single point. */
    CGeneralGeometry(const point_t& aPoint,TCoordType aCoordType):
        m_coord_type(aCoordType),
        m_closed(false)
        {
        AppendPoint(aPoint);
        }
    /** Creates a geometry object containing the geometry of a map object. The geometry object is closed is the map object is a polygon or array (i.e., a texture), otherwise open. */
    explicit CGeneralGeometry(const CMapObject& aMapObject):
        CGeneralGeometry(aMapObject,TCoordType::Map,aMapObject.Type() == TMapObjectType::Polygon || aMapObject.Type() == TMapObjectType::Array)
        {
        }
    /** Returns a COutline object containing this geometry. */
    operator COutline() const
        {
        COutline outline;
        TOutlinePoint p;
        for (const auto& contour : m_contour_array)
            {
            CContour& c = outline.AppendContour();
            c.SetClosed(m_closed);
            for (const auto& point: contour)
                {
                TOutlinePointFP q { point };
                p.iX = Round(q.iX);
                p.iY = Round(q.iY);
                p.iType = q.iType;
                c.AppendPoint(p);
                }
            }
        return outline;
        }
    /** Sets the object to its just-constructed state: open, empty, and using map coordinates. */
    void Clear() { m_contour_array.resize(1); m_contour_array[0].resize(0); m_coord_type = TCoordType::Map; m_closed = false; }
    /** Returns the coordinate type used for all points in this geometry. */
    TCoordType CoordType() const { return m_coord_type; }
    /** Returns the number of contours. */
    size_t ContourCount() const { return m_contour_array.size(); }
    /** Returns the number of points in a given contour. */
    size_t PointCount(size_t aContourIndex) const { return m_contour_array[aContourIndex].size(); }
    /** Returns a point identified by its contour index and point index. */
    const point_t& Point(size_t aContourIndex,size_t aPointIndex) const { return m_contour_array[aContourIndex][aPointIndex]; }
    /** Returns a non-const reference to a point identified by its contour index and point index. */
    point_t& Point(size_t aContourIndex,size_t aPointIndex) { return m_contour_array[aContourIndex][aPointIndex]; }
    /** Returns true if the geometry is empty. */
    bool IsEmpty() const { return m_contour_array[0].empty(); }
    /** Returns true if the geometry is closed (formed of closed paths). */
    bool IsClosed() const { return m_closed; }
    /** Sets the open/closed status. */
    void SetClosed(bool aClosed) { m_closed = aClosed; }
    /** Returns a writable coordinate set referring to this geometry. */
    TWritableCoordSet CoordSet(size_t aContourIndex)
        {
        auto& c = m_contour_array[aContourIndex];
        if (!c.size())
            return TWritableCoordSet();
        return TWritableCoordSet(&c[0].iX,&c[0].iY,sizeof(point_t),c.size());
        }
    /** Returns a coordinate set referring to this geometry. */
    TCoordSet CoordSet(size_t aContourIndex) const
        {
        auto& c = m_contour_array[aContourIndex];
        if (!c.size())
            return TCoordSet();
        return TCoordSet(&c[0].iX,&c[0].iY,sizeof(point_t),c.size());
        }
    /** Returns the bounds as an axis-aligned rectangle. */
    TRectFP Bounds() const
        {
        TRectFP bounds;
        if (m_contour_array[0].size())
            bounds.iTopLeft = bounds.iBottomRight = m_contour_array[0][0];
        for (const auto& c : m_contour_array)
            for (const auto& p : c)
                bounds.Combine(p);
        return bounds;
        }
    /** Appends a point to the current (last) contour in this geometry. */
    void AppendPoint(const point_t& aPoint) { m_contour_array.back().push_back(aPoint); }
    /** Appends a point to the last contour in this geometry. */
    void AppendPoint(double aX,double aY) { m_contour_array.back().push_back(point_t(aX,aY)); }
    /** Appends a point to the last contour in this geometry, specifying the point type. */
    void AppendPoint(double aX,double aY,TPointType aPointType) { m_contour_array.back().push_back(point_t(aX,aY,aPointType)); }
    /** Starts a new contour. Does nothing if the current (last) contour is empty. */
    void BeginContour() { if (!m_contour_array.back().empty()) m_contour_array.emplace_back(); }
    /** Reverses the order of the contours and the order of the points in each contour. */
    void Reverse()
        {
        std::reverse(m_contour_array.begin(),m_contour_array.end());
        for (auto& c : m_contour_array)
            std::reverse(c.begin(),c.end());
        }
    /** Converts the coordinates to aToCoordType using the function aConvertFunction. For internal use only. */
    TResult ConvertCoords(TCoordType aToCoordType,std::function<TResult(TWritableCoordSet& aCoordSet)> aConvertFunction)
        {
        if (m_coord_type == aToCoordType)
            return KErrorNone;
        m_coord_type = aToCoordType;
        size_t contour_count = ContourCount();
        for (size_t i = 0; i < contour_count; i++)
            {
            TWritableCoordSet cs { CoordSet(i) };
            TResult error = aConvertFunction(cs);
            if (error)
                return error;
            }
        return KErrorNone;
        }

    private:
    using contour_t = std::vector<point_t>;
    std::vector<contour_t> m_contour_array = std::vector<contour_t>(1);
    TCoordType m_coord_type = TCoordType::Map;
    bool m_closed = false;
    };

/** A geometry class for creating map objects and specifying view areas. */
class CGeometry: public CGeneralGeometry<TOutlinePointFP>
    {
    using CGeneralGeometry::CGeneralGeometry; // inherit constructors
    };

} // namespace CartoType

#endif // #define CARTOTYPE_GEOMETRY__
