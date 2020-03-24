package com.cartotype.testgl;

import android.Manifest;
import android.app.Fragment;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.pm.ApplicationInfo;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.support.v4.content.ContextCompat;
import android.support.v4.app.FragmentManager;
import android.util.Log;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;

import com.cartotype.*;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class MainActivity extends AppCompatActivity
    {
    class MyFindHelper implements FindAsyncInterface
        {
        public void handler(MapObject[] aMapObjectArray)
            {
            m_found_map_object_array = aMapObjectArray;
            }
        }

    class MyFindGroupHelper implements FindAsyncGroupInterface
        {
        public void handler(MapObjectGroup[] aMapObjectGroupArray)
            {
            m_found_map_object_group_array = aMapObjectGroupArray;
            }
        }

    class MyRouterHelper implements RouterAsyncInterface
        {
        public void handler(int aError,Route aRoute)
            {
            m_route = aRoute;
            }
        }

    @Override
    protected void onCreate(Bundle savedInstanceState)
        {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        //FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        //fab.setOnClickListener(new View.OnClickListener() {
        //    @Override
        //    public void onClick(View view) {
        //        Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
        //                .setAction("Action", null).show();
        //    }
        //});


            int permission = ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE);

            File map_file, font_file, style_file;

            try
                {
                map_file = getCopyOfFile(this, "data/isle_of_wight.ctm1", "isle_of_wight.ctm1");
                font_file = getCopyOfFile(this, "data/DejaVuSans.ttf", "DejaVuSans.ttf");
                style_file = getCopyOfFile(this, "data/standard.ctstyle", "standard.ctstyle");
                }
            catch (Exception e)
                {
                Log.d("CartoType","failed to open map, font or style sheet file");
                return;
                }

            // String ct_dir = Environment.getExternalStorageDirectory().getPath() + "/Documents/CartoType/";
            // String ct_dir = getFilesDir().getPath() + "/CartoType/";
            // String map_file_name = ct_dir + "maps/britain_and_ireland.ctm1";
            //String style_file = ct_dir + "style/standard.ctstyle";
            //String font_file = ct_dir + "font/DejaVuSans.ttf";

            final Framework framework = new Framework(this,map_file.getAbsolutePath(),style_file.getAbsolutePath(),font_file.getAbsolutePath(),
                    256,256);

            m_view = new MainView(this,framework);
            setContentView(m_view);

            // Set the scale and create a scale bar after creating the main view, which creates a MapView which sets the screen resolution.
            //framework.setScale(500000);
            framework.setScaleBar(true,3,"in",NoticePosition.BottomLeft);

            framework.setNightMode(true);

            // Create a route.
            //int error = framework.startNavigation(-1.56815,50.6666,CoordType.Degree,-1.070261,50.684697,CoordType.Degree);
            /***
            int n = framework.builtInProfileCount();
            framework.setBuiltInProfile(1);
            RouteProfile rp = framework.builtInProfile(1);

            RouteCoordSet cs = new RouteCoordSet();
            cs.iCoordType = CoordType.Degree;
            cs.iRoutePointArray = new RoutePoint[2];
            cs.iRoutePointArray[0] = new RoutePoint();
            cs.iRoutePointArray[0].iX = -2.7616;
            cs.iRoutePointArray[0].iY = 51.4273;
            cs.iRoutePointArray[0].iHeading = 45;
            cs.iRoutePointArray[0].iHeadingKnown = true;
            cs.iRoutePointArray[0].iLocationMatchParam = new LocationMatchParam();
            cs.iRoutePointArray[0].iLocationMatchParam.iHeadingAccuracyInDegrees = 5;
            cs.iRoutePointArray[1] = new RoutePoint();
            cs.iRoutePointArray[1].iX = -1.8578;
            cs.iRoutePointArray[1].iY = 51.4148;
            int error = framework.startNavigation(cs);
             ***/

            //int error = framework.startNavigation(-2.7616,51.4273,CoordType.Degree,-1.8578,51.4148,CoordType.Degree);


            // m_route = framework.getRoute(0);

            // Display the app build date as if it was a copyright notice.
            // String b = framework.appBuildDate();
            /// framework.setCopyrightNotice(b);

            // Display turn instructions.
            framework.setTurnInstructions(true,false, 3, "in", NoticePosition.TopLeft, 12, "pt");

            // Create a timer to move along the route for the first ten kilometres.
            /******
            framework.setFollowMode(Framework.FOLLOW_MODE_LOCATION);
            m_timer_handler = new Handler();
            m_timer_runnable = new Runnable()
            {
                @Override
                public void run()
                {
                    NearestSegmentInfo info = m_route.getPointAtDistance(m_route_pos);
                    double[] coord = new double[2];
                    coord[0] = info.iNearestPointX;
                    coord[1] = info.iNearestPointY;
                    framework.convertCoords(coord,CoordType.Map,CoordType.Degree);
                    framework.navigate(Framework.POSITION_VALID,0,coord[0],coord[1],0,0,0);
                    m_route_pos += 10;
                    if (m_route_pos < 10000)
                        m_timer_handler.postDelayed(this,500);
                }
            };

            m_timer_handler.postDelayed(m_timer_runnable,0);
             /******/


        }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

        private File getCopyOfFile(Context aContext, String aAssetPath, String aAssetFile) throws IOException
        {
            AssetManager am = aContext.getAssets();
            ContextWrapper wrapper = new ContextWrapper(aContext);
            File dir =  wrapper.getDir("localAssets",Context.MODE_PRIVATE);
            File f = new File(dir,aAssetFile);

            // Copy the file if it doesn't exist.
            if (!f.exists())
            {
                InputStream is = am.open(aAssetPath);
                OutputStream os = new FileOutputStream(f);
                byte[] buffer = new byte[1024];
                for (; ; )
                {
                    int length = is.read(buffer);
                    if (length <= 0)
                        break;
                    os.write(buffer, 0, length);
                }
                os.flush();
                os.close();
                is.close();
            }

            return f;
        }

    private static PathPoint PointOnPath(Path aPath,double aPos)
        {
            PathPoint point = new PathPoint();
            PathPoint prev_point = new PathPoint();
            PathPoint cur_point = new PathPoint();

            double length = 0;
            int n = aPath.getContourCount();
            if (aPos < 0)
                aPos = 0;
            for (int contour_index = 0; contour_index < n; contour_index++)
            {
                int point_count = aPath.getPointCount(contour_index);
                if (point_count < 2)
                    continue;

                aPath.getPoint(contour_index,0,prev_point);
                for (int point_index = 1; point_index < point_count; point_index++)
                {
                    aPath.getPoint(contour_index,point_index,cur_point);
                    double dx = cur_point.iX - prev_point.iX;
                    double dy = cur_point.iY - prev_point.iY;
                    double cur_length = Math.sqrt(dx * dx + dy * dy);
                    length += cur_length;
                    if (length >= aPos)
                    {
                        if (cur_length > 0)
                        {
                            double factor = (length - aPos) / cur_length;
                            point.iX = cur_point.iX - dx * factor;
                            point.iY = cur_point.iY - dy * factor;
                        }
                        else
                        {
                            point.iX = prev_point.iX;
                            point.iY = prev_point.iY;
                        }
                        return point;
                    }

                    prev_point = cur_point;
                }
            }

            return cur_point;
        }


    private MainView m_view;
    private Handler m_timer_handler;
    private Runnable m_timer_runnable;
    private Route m_route;
    private double m_route_pos;
    private MapObject[] m_found_map_object_array;
    private MyFindHelper m_find_helper = new MyFindHelper();
    private MapObjectGroup[] m_found_map_object_group_array;
    private MyFindGroupHelper m_find_group_helper = new MyFindGroupHelper();
    private MyRouterHelper m_router_helper = new MyRouterHelper();
    }
