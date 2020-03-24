package com.cartotype.testgl;

import android.content.Context;
import com.cartotype.*;

public class MainView extends MapView
    {
    MainView(Context aContext,Framework aFramework)
        {
        super(aContext,aFramework);
        m_framework = aFramework;
        }

    public void onTap(double aX,double aY)
        {
            m_start[0] = m_end[0];
            m_start[1] = m_end[1];
            m_end[0] = aX;
            m_end[1] = aY;

            if (m_start[0] != 0 && m_end[0] != 0)
                m_framework.startNavigation(m_start[0],m_start[1],CoordType.Degree,m_end[0],m_end[1],CoordType.Degree);

        }

    private Framework m_framework;
        private double[] m_start = new double[2];
        private double[] m_end = new double[2];
    }
