/**
 * @file
 * @author  Robbie Cao<hjcao@memsic.cn>
 *
 * @brief
 * This file implement a very simple sensor application to display
 * sensor data on screen. 
 */

package com.memsic.SensorTest;

import android.app.Activity;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.hardware.SensorManager;
import android.hardware.SensorListener;

public class SensorTest extends Activity implements SensorListener {
	final String tag = "SensorTest";
	SensorManager sm = null;
	TextView xViewA = null;
	TextView yViewA = null;
	TextView zViewA = null;
	TextView xViewO = null;
	TextView yViewO = null;
	TextView zViewO = null;
	TextView xViewM = null;
	TextView yViewM = null;
	TextView zViewM = null;
	 
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        sm = (SensorManager) getSystemService(SENSOR_SERVICE);
        setContentView(R.layout.main);
        xViewO = (TextView) findViewById(R.id.xboxo);
        yViewO = (TextView) findViewById(R.id.yboxo);
        zViewO = (TextView) findViewById(R.id.zboxo);
        xViewA = (TextView) findViewById(R.id.xboxa);
        yViewA = (TextView) findViewById(R.id.yboxa);
        zViewA = (TextView) findViewById(R.id.zboxa);
        xViewM = (TextView) findViewById(R.id.xboxm);
        yViewM = (TextView) findViewById(R.id.yboxm);
        zViewM = (TextView) findViewById(R.id.zboxm);
    }
    
    
    public void onSensorChanged(int sensor, float[] values) {
        synchronized (this) {
            Log.d(tag, "onSensorChanged: " + sensor + "X: " + values[0] + ", Y: " + values[1] + ", Z: " + values[2]);
            // See "Definition of the axis" in sensors.h for detail
            if (sensor == SensorManager.SENSOR_ACCELEROMETER) {
                xViewA.setText("\tX\t\t: " + values[0]);
	        yViewA.setText("\tY\t\t: " + values[1]);
	        zViewA.setText("\tZ\t\t: " + values[2]);
            }
            if (sensor == SensorManager.SENSOR_MAGNETIC_FIELD) {
                xViewM.setText("\tX\t\t: " + values[0]);
	        yViewM.setText("\tY\t\t: " + values[1]);
	        zViewM.setText("\tZ\t\t: " + values[2]);
            }  
            if (sensor == SensorManager.SENSOR_ORIENTATION) {
                xViewO.setText("\tAzimuth: " + values[0]);
	        yViewO.setText("\tPitch\t: " + values[1]);
	        zViewO.setText("\tRoll\t\t: " + values[2]);
            }  
        }
    }

    public void onAccuracyChanged(int sensor, int accuracy) {
    	Log.d(tag, "onAccuracyChanged: " + "sensor: " + sensor + ", accuracy: " + accuracy);
    }
 

    @Override
    protected void onResume() {
        super.onResume();
        sm.registerListener(this, 
                SensorManager.SENSOR_ORIENTATION 
                	| SensorManager.SENSOR_ACCELEROMETER 
                	| SensorManager.SENSOR_MAGNETIC_FIELD,
                SensorManager.SENSOR_DELAY_FASTEST); // interval
    }
    
    @Override
    protected void onStop() {
        sm.unregisterListener(this);
        super.onStop();
    }    
    
    
}
