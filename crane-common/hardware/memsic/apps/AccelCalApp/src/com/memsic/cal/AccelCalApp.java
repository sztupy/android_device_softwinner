/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information and source code
 *  contained herein is confidential. The software including the source code
 *  may not be copied and the information contained herein may not be used or
 *  disclosed except with the written permission of MEMSIC Inc. (C) 2009
 *****************************************************************************/

package com.memsic.cal;

import android.app.Activity;
import android.widget.TextView;
import android.os.Bundle;

public class AccelCalApp extends Activity
{
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        TextView  tv = new TextView(this);

        // here, we dynamically load the library at runtime
        // before calling the native method.
        //
        System.loadLibrary("accelcal_jni");

        int  z = calibrate();

        tv.setText("Accelerometer Calibrate Result: " + ((z == 0) ? "Fail" : "Success"));
        setContentView(tv);
    }

    public native int calibrate();
}
