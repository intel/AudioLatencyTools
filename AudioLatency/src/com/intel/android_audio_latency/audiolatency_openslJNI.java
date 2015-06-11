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

public class audiolatency_openslJNI {

    static {
	try {
	    java.lang.System.loadLibrary("audiolatency_opensles");
	} catch (UnsatisfiedLinkError e) {
	    java.lang.System.err.println("native code library failed to load.\n" + e);
	    java.lang.System.exit(1);
	}
    }

    public static void start_roundtrip_process() {
	start_roundtrip();
    }

    public static void stop_roundtrip_process() {
	stop_roundtrip();
    }

    public static void start_roundtrip_google_process() {
	start_roundtrip_google();
    }

    public static void stop_roundtrip_google_process() {
	stop_roundtrip_google();
    }

    public static void start_input_process() {
	start_input();
    }

    public static void stop_input_process() {
	stop_input();
    }

    public static void start_output_process() {
	start_output();
    }

    public static void stop_output_process() {
	stop_output();
    }


    public static void start_continuous_output_process() {
	start_continuous_output();
    }

    public static void stop_continuous_output_process() {
	stop_continuous_output();
    }

    public static void start_continuous_input_process() {
	start_continuous_input();
    }

    public static void stop_continuous_input_process() {
	stop_continuous_input();
    }

    public static void set_audio_parameters_java(int bufferFrames, int sampleRate) {
	set_audio_parameters(bufferFrames, sampleRate);
    }

    public static void start_usb2gpio_calibration_process() {
	start_usb2gpio_calibration();
    }

    public static void stop_usb2gpio_calibration_process() {
	stop_usb2gpio_calibration();
    }

    public final static native void start_roundtrip();
    public final static native void stop_roundtrip();
    public final static native void start_roundtrip_google();
    public final static native void stop_roundtrip_google();
    public final static native void start_input();
    public final static native void stop_input();
    public final static native void start_output();
    public final static native void stop_output();
    public final static native void start_continuous_output();
    public final static native void stop_continuous_output();
    public final static native void start_continuous_input();
    public final static native void stop_continuous_input();
    public final static native void set_audio_parameters(int bufferFrames, int sampleRate);
    public final static native void start_usb2gpio_calibration();
    public final static native void stop_usb2gpio_calibration();


}
