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

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.media.AudioManager;
import java.lang.Integer;


public class AudiolatencytestActivity extends Activity {
    /** Called when the activity is first created. */
    private final String TAG = "OpenSLES-AudioLatency-JAVA";
    private final int TESTCASE = 7;
    private int currentCase = -1; // 0-TESTCASE-1
    private Thread thread;
    private Button coldOutputButton;
    private Button coldInputButton;
    private Button roundTripButton;
    private Button continuousOutputButton;
    private Button continuousInputButton;
    private Button USB2GPIOButton;
    private Button roundTripGoogleButton;
    private boolean status[];
    private Button buttons[];
        
    private PulseUSB usb;

    private int bufferFrames = 1024;
    private int sampleRate = 44100;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        status = new boolean[TESTCASE];
        buttons = new Button[TESTCASE];

        initView();
        setButtonStatus(-1);
        initUSB();
        // To get preferred buffer size and sampling rate
        AudioManager audioManager = (AudioManager)this.getSystemService(Context.AUDIO_SERVICE);
        String size = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        String rate = audioManager.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        if (size != null) {
            bufferFrames = Integer.parseInt(size);
        }

        if (rate != null) {
            sampleRate = Integer.parseInt(rate);
        }
        audiolatency_openslJNI.set_audio_parameters(bufferFrames, sampleRate);
    }
    

    @Override
    public void onBackPressed() {
	super.onBackPressed();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
	// TODO Auto-generated method stub
	if (keyCode == KeyEvent.KEYCODE_BACK) {
	    Log.d(TAG, "back!!");

            if (currentCase >= 0 && currentCase < TESTCASE) {
                for (int i =0; i<TESTCASE; i++) {
                    if (i==currentCase) {
                        buttons[i].setText("Start");
                        status[i] = false;
                        stopThread(i);
                    } else {
                        buttons[i].setEnabled(true);
                    }
                }
            }

            currentCase = -1;

            //this.finish();
	    android.os.Process.killProcess(android.os.Process.myPid());
	    return true;
	}
	if (keyCode == KeyEvent.KEYCODE_MENU) {
	    return true;
	}
	return super.onKeyDown(keyCode, event);
    }

    public void initUSB() {
    	usb = new PulseUSB(this);
    	usb.listDevices();    	
    }

    private void startThread(int caseNumber) {
        final int currentNumber = caseNumber;
        thread = new Thread() {
		public void run() {
		    setPriority(Thread.MAX_PRIORITY);
                    currentCase = currentNumber;
		    if (currentNumber == 0) {
			audiolatency_openslJNI.start_output_process();
		    } else if (currentNumber == 1) {
			audiolatency_openslJNI.start_input_process();
		    } else if (currentNumber == 2) {
			audiolatency_openslJNI.start_roundtrip_process();
		    } else if (currentNumber == 3) {
			audiolatency_openslJNI.start_continuous_output_process();
		    } else if (currentNumber == 4) {
			audiolatency_openslJNI.start_continuous_input_process();
		    } else if (currentNumber == 5) {
			audiolatency_openslJNI.start_usb2gpio_calibration_process();
		    } else if (currentNumber == 6) {
			audiolatency_openslJNI.start_roundtrip_google_process();
		    }
		}
	    };
        thread.start();
    }
    private void stopThread(int caseNumber) {
        if (caseNumber == 0) {
            audiolatency_openslJNI.stop_output_process();
        } else if (caseNumber == 1) {
            audiolatency_openslJNI.stop_input_process();
        } else if (caseNumber == 2) {
            audiolatency_openslJNI.stop_roundtrip_process();
        } else if (caseNumber == 3) {
            audiolatency_openslJNI.stop_continuous_output_process();
        } else if (caseNumber == 4) {
            audiolatency_openslJNI.stop_continuous_input_process();
        } else if (caseNumber == 5) {
            audiolatency_openslJNI.stop_usb2gpio_calibration_process();
        } else if (caseNumber == 6) {
            audiolatency_openslJNI.stop_roundtrip_google_process();
        } else {
            return;
        }
        currentCase = -1;

        if (thread != null) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            thread = null;
        }
    }
    private void setButtonStatus(int position) {
        if (position == -1) { // init button
            buttons[0] = coldOutputButton;
            buttons[1] = coldInputButton;
            buttons[2] = roundTripButton;
            buttons[3] = continuousOutputButton;
            buttons[4] = continuousInputButton;
            buttons[5] = USB2GPIOButton;
            buttons[6] = roundTripGoogleButton;

            for (int i =0; i<TESTCASE; i++) {
                status[i] = false;
                buttons[i].setEnabled(true);
            }
        }

        if (0 <= position && position < TESTCASE) { // click coldOutputButton
            if (status[position] == false) {
                for (int i = 0; i<TESTCASE; i++) {
                    if (i == position) {
                        buttons[i].setText("Stop");
                        status[i] = true;
                    } else {
                        buttons[i].setEnabled(false);
                    }
                }
            } else {
                for (int i =0; i<TESTCASE; i++) {
                    if (i==position) {
                        buttons[i].setText("Start");
                        status[i] = false;
                    } else {
                        buttons[i].setEnabled(true);
                    }
                }
            }
        }
    }

    private void startTestCase(int caseNumber) {
        setButtonStatus(caseNumber);
        if (status[caseNumber] == true) {
            startThread(caseNumber);
        } else {
            stopThread(caseNumber);
        }
    }
    private void initView() {
        coldOutputButton = (Button) findViewById(R.id.cold_output);
        coldInputButton = (Button) findViewById(R.id.cold_input);
        roundTripButton = (Button) findViewById(R.id.roundtrip);
        continuousOutputButton = (Button) findViewById(R.id.continuous_output);
        continuousInputButton = (Button) findViewById(R.id.continuous_input);
        USB2GPIOButton = (Button) findViewById(R.id.usb2gpio_calibration);
        roundTripGoogleButton = (Button) findViewById(R.id.roundtrip_google);

        coldOutputButton.setOnClickListener(new OnClickListener() {
		@Override
		public void onClick(View arg0) {
		    startTestCase(0);
		}
	    });

        coldInputButton.setOnClickListener(new OnClickListener() {
		@Override
		public void onClick(View arg0) {
		    startTestCase(1);
		}
	    });

        roundTripButton.setOnClickListener(new OnClickListener() {
		@Override
		public void onClick(View arg0) {
		    startTestCase(2);
		}
	    });

	continuousOutputButton.setOnClickListener(new OnClickListener() {
		@Override
		public void onClick(View arg0) {
		    startTestCase(3);
		}
	    });

	continuousInputButton.setOnClickListener(new OnClickListener() {
		@Override
		public void onClick(View arg0) {
		    startTestCase(4);
		}
	    });

	USB2GPIOButton.setOnClickListener(new OnClickListener() {
		@Override
		public void onClick(View arg0) {
		    startTestCase(5);
		}
	    });
        roundTripGoogleButton.setOnClickListener(new OnClickListener() {
		@Override
		public void onClick(View arg0) {
		    startTestCase(6);
		}
	    });


    }

    public void onDestroy(){
    	
    	super.onDestroy();

        Log.d(TAG, "onDestroy!!");
        if (currentCase >= 0 && currentCase < TESTCASE) {
            for (int i =0; i<TESTCASE; i++) {
                if (i==currentCase) {
                    buttons[i].setText("Start");
                    status[i] = false;
                    stopThread(i);
                } else {
                    buttons[i].setEnabled(true);
                }
            }
        }
        currentCase = -1;
    }
    
}
