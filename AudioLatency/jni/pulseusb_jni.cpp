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
//#include <android_runtime/AndroidRuntime.h>
#include "jni.h"
//#include "JNIHelp.h"
#include "pulseusb_jni.h"
#include <stdio.h>
#include <stdlib.h>

#define LOG_TAG "PulseUSB_JNI"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERRO,LOG_TAG,__VA_ARGS__)

#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

static const char* classname = "com/intel/android_audio_latency/PulseUSB";
static jobject gUsb = NULL;
static jmethodID gMethodId;
static JNIEnv* gEnv;
static JavaVM* jVm;
static void
native_init(JNIEnv *env, jobject thiz, jobject usb)
{
    LOGD("native init\n");
    gUsb = env->NewGlobalRef(usb);
    jclass clazz = env->FindClass(classname);
    if (clazz == NULL)
        LOGD("Class: %s is not found.\n", classname);

    gMethodId = env->GetMethodID(clazz, "setDtr", "(I)V");
    if (gMethodId == NULL)
        LOGD("Method: setDtr is not found.\n");

    env->CallVoidMethod(thiz, gMethodId, 0);

}

void
native_setDtr(int dtr)
{
    LOGD("native setDtr: %d\n", dtr);

    if (gUsb == NULL) {
        LOGW("gUsb is null\n");
        return;
    }

    if (gMethodId == NULL) {
        LOGW("gMethodId is null\n");
        return;
    }
    JNIEnv* env;
    jVm->AttachCurrentThread(&env, NULL);
    if(env == NULL) {
        LOGW("env is null\n");
        return;
    }
    env->CallVoidMethod(gUsb, gMethodId, dtr);
    //jVm->DetachCurrentThread();
}


static JNINativeMethod methods[] = {
    {"native_init", "(Ljava/lang/Object;)V", (void *)native_init}
};

int register_method(JNIEnv* env)                                                                                 {
    jclass clazz;
    clazz = env->FindClass(classname);
    return env->RegisterNatives(
               clazz, methods, NELEM(methods));
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;

    if (!vm) {
        LOGD("ERROR: vm is NULL");
        goto fail;
    }
    jVm = vm;
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGD("ERROR: GetEnv failed\n");
        goto fail;
    }
    if (register_method(env) < 0) {
        LOGD("ERROR: failed to registe pulseusb_jni method\n");
        goto fail;
    }

    LOGD("jni onload success");
    result = JNI_VERSION_1_4;

fail:
    return result;

}


