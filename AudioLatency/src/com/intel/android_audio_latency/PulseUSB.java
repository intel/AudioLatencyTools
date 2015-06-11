/*
 * Copyright (C) 2015 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.intel.android_audio_latency;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Iterator;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.util.Log;


public class PulseUSB {
    private static final String TAG = "PulseUSB";
    private static final String FTDI_VID_PID = "0403:6001";
    private static UsbManager mUsbManager = null;
    private static UsbDevice mUsbDevice = null; // will be set on permission granted
    private static UsbDeviceConnection mConnection = null;
    private static boolean hasRegisted = false;
    private Context mContext;
    
    static {
        try {
            java.lang.System.loadLibrary("pulseusb");
        } catch (UnsatisfiedLinkError e) {
            java.lang.System.err.println("native code library failed to load.\n" + e);
            java.lang.System.exit(1);
        }
    }
    
    public PulseUSB (Context context) {
	mContext = context;
	native_init(this);
    }
   
    public native void native_init(Object o);

	   
    private void showLog(String st) {
    	Context context = mContext;
    	//Toast toast = Toast.makeText(context, st, Toast.LENGTH_SHORT);
    	//toast.show();
    	Log.d(TAG, st);
    }
    
    private static final String ACTION_USB_PERMISSION =
	"com.intel.audiolatency.USB_PERMISSION";
    private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
	    public void onReceive(Context context, Intent intent) {
		String action = intent.getAction();
		if (ACTION_USB_PERMISSION.equals(action)) {
		    synchronized (this) {
			UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

			if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
			    if(device != null){
				//call method to set up device communication
				showLog("permission granted");
				connectionSetup(device);
			    }
			} else {
			    showLog("permission denied for device " + device);
			}
		    }
		}
	    }
	};

    public int listDevices() {
    	Log.v(TAG, "listDevices");
    	mUsbManager = (UsbManager)mContext.getSystemService(mContext.USB_SERVICE);
    	HashMap<String, UsbDevice> deviceList = mUsbManager.getDeviceList();
    	Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
    	
    	while (deviceIterator.hasNext()) {
	    UsbDevice d = deviceIterator.next();
	    if (mUsbManager.hasPermission(d)) {
		showLog("Permission for device already granted");
		connectionSetup(d);
	    } else {
		PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0, new Intent(ACTION_USB_PERMISSION), 0);
		IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
		mContext.registerReceiver(mUsbReceiver, filter);
		hasRegisted = true;
		mUsbManager.requestPermission(d, pendingIntent);
	    }
    	}
    	return deviceList.size();
    }

    private void connectionSetup(UsbDevice d) {
    	mUsbDevice = d;
    	
    	if (mUsbDevice == null)
	    return;
    	
    	mConnection = mUsbManager.openDevice(mUsbDevice);
    	showLog("Interface Count: " + mUsbDevice.getInterfaceCount() + "for device" +
		String.format("%04X:%04X", mUsbDevice.getVendorId(), mUsbDevice.getProductId()));
    	
    	if (!mConnection.claimInterface(mUsbDevice.getInterface(0), true)) {
	    showLog("claimInterface failed");
	    return;
    	}
    	
    	showLog("connection worked, default state 0");
    	setDtr(0);
    }
    
    public void setDtr(int state) {
    	// reverse-engineered from libftdi runs
    	// toggling dtr to 0: requestType 0x40 request 0x1 value 256 index 1 timeout 5000
    	// toggling dtr to 1: requestType 0x40 request 0x1 value 257 index 1 timeout 5000
    	int usb_val;
    	if (state == 0)
	    usb_val = 256;
    	else
	    usb_val = 257;
    	
    	showLog("DTR state = " + state);
    	if (mConnection == null) {
	    Log.d(TAG, "mConnection is null");
	    return;
    	}
    	mConnection.controlTransfer(
				    0x40, //int requestType,
				    0x1, // int request,
				    usb_val, // int value,
				    1,   // int index,
				    null, // byte[] buffer,
				    0, // int length,
				    0 // int timeout)
				    );
    	
    	showLog("state change done");
    }
}
