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

#include <android/log.h>

#include "jni.h"
#include <stdio.h>
#include <stdlib.h>
#include "opensles_audiolatency.h"
#include "pulseusb_jni.h"
#define LOG_TAG "OpenslESJNI"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
//using namespace android;
#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

JavaVM* PCGlobalVM = NULL;
int register_opensl_audiolatency_jni(JNIEnv *env);

static const char* const kClassOpenslesLatency =
    "com/intel/android_audio_latency/audiolatency_openslJNI";
static const char* const kRunTimeException =
    "java/lang/RuntimeException";
static const char* const kIllegalArgumentException =
    "java/lang/IllegalArgumentException";

//jni onload
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    PCGlobalVM = vm;
    JNIEnv* env = NULL;
    jint result = -1;

    if (!vm) {
        LOGE("ERROR: vm is NULL");
        goto bail;
    }
    PCGlobalVM = vm;
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    if (register_opensl_audiolatency_jni(env) < 0) {
        LOGE("ERROR: v2ip video engine native registration failed\n");
        goto bail;
    }

    LOGE("jni onload success");
    //success -- return valid version number
    result = JNI_VERSION_1_4;

bail:
    return result;
}

static void
opensl_audiolatency_start_roundtrip(JNIEnv *env, jobject thiz)
{
    LOGD("start roundtrip");
    start_roundtrip();
}


static void
opensl_audiolatency_stop_roundtrip(JNIEnv *env, jobject thiz)
{
    LOGD("stop roundtrip");
    stop_roundtrip();
}

static void
opensl_audiolatency_start_roundtrip_google(JNIEnv *env, jobject thiz)
{
    LOGD("start roundtrip");
    start_roundtrip_google();
}


static void
opensl_audiolatency_stop_roundtrip_google(JNIEnv *env, jobject thiz)
{
    LOGD("stop roundtrip");
    stop_roundtrip_google();
}

static void
opensl_audiolatency_start_input(JNIEnv *env, jobject thiz)
{
    LOGD("start input");
    start_input();
}


static void
opensl_audiolatency_stop_input(JNIEnv *env, jobject thiz)
{
    LOGD("stop input");
    stop_input();
}

static void
opensl_audiolatency_start_output(JNIEnv *env, jobject thiz)
{
    LOGD("start output");
    start_output();
}


static void
opensl_audiolatency_stop_output(JNIEnv *env, jobject thiz)
{
    LOGD("stop output");
    stop_output();
}

static void
opensl_audiolatency_start_continuous_output(JNIEnv *env, jobject thiz)
{
    LOGD("start continuous output");
    start_continuous_output();
}


static void
opensl_audiolatency_stop_continuous_output(JNIEnv *env, jobject thiz)
{
    LOGD("stop continuous output");
    stop_continuous_output();
}

static void
opensl_audiolatency_start_continuous_input(JNIEnv *env, jobject thiz)
{
    LOGD("start continuous input");
    start_continuous_input();
}


static void
opensl_audiolatency_stop_continuous_input(JNIEnv *env, jobject thiz)
{
    LOGD("stop continuous input");
    stop_continuous_input();
}

static void
opensl_audiolatency_set_audio_parameters(JNIEnv * env, jobject thiz, jint bufferFrames, jint sampleRate) {

    LOGD("set audio parameters");
    /** Double validation... set which one you want to follow Java or C **/
    if(bufferFrames>0 && sampleRate>0) {
        set_audio_parameters(bufferFrames, sampleRate);
    }
}

static void
opensl_audiolatency_start_usb2gpio_calibration(JNIEnv *env, jobject thiz)
{
    LOGD("start_usb2gpio_calibration");
    start_usb2gpio_calibration();
}
static void
opensl_audiolatency_stop_usb2gpio_calibration(JNIEnv *env, jobject thiz)
{
    LOGD("stop_usb2gpio_calibration");
    stop_usb2gpio_calibration();
}

//gmethods functions
static JNINativeMethod gMethods[] = {
    { "start_roundtrip", "()V", (void *)opensl_audiolatency_start_roundtrip},
    { "stop_roundtrip", "()V", (void *)opensl_audiolatency_stop_roundtrip},
    { "start_roundtrip_google", "()V", (void *)opensl_audiolatency_start_roundtrip_google},
    { "stop_roundtrip_google", "()V", (void *)opensl_audiolatency_stop_roundtrip_google},
    { "start_input", "()V", (void *)opensl_audiolatency_start_input},
    { "stop_input", "()V", (void *)opensl_audiolatency_stop_input},
    { "start_output", "()V", (void *)opensl_audiolatency_start_output},
    { "stop_output", "()V", (void *)opensl_audiolatency_stop_output},
    { "start_continuous_output", "()V", (void *)opensl_audiolatency_start_continuous_output},
    { "stop_continuous_output", "()V", (void *)opensl_audiolatency_stop_continuous_output},
    { "start_continuous_input", "()V", (void *)opensl_audiolatency_start_continuous_input},
    { "stop_continuous_input", "()V", (void *)opensl_audiolatency_stop_continuous_input},
    { "set_audio_parameters", "(II)V", (void *)opensl_audiolatency_set_audio_parameters},
    { "start_usb2gpio_calibration", "()V", (void *)opensl_audiolatency_start_usb2gpio_calibration},
    { "stop_usb2gpio_calibration", "()V", (void *)opensl_audiolatency_stop_usb2gpio_calibration},

};

int register_opensl_audiolatency_jni(JNIEnv *env)
{
    LOGD("register opensl latency jni");
    jclass clazz;
    clazz = env->FindClass(kClassOpenslesLatency);
    return env->RegisterNatives(
               clazz, gMethods, NELEM(gMethods));
}

