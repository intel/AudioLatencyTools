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


#ifndef OPENSLES_BASIC
#define OPENSLES_BASIC

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <pthread.h>
#include <stdlib.h>
#include "buffer_lock.h"

#include<android/log.h>
#define LOG_TAG "OpenSL-AudioLatency"
#define LOGV(fmt, args...) __android_log_print(ANDROID_LOG_VERBOSE,  LOG_TAG, fmt, ##args)
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    // buffers
    short *outputBuffer[2];
    short *inputBuffer[2];

    // buffer indexes
    int currentInPositionIndex;
    int currentOutPositionIndex;
    int currentInBufferIndex;
    int currentOutBufferIndex;


    // locks
    void*  inBufferLock;
    void*  outBufferLock;

    // audio parameters
    int sampleRate;
    int inBufSamples;
    int outBufSamples;
    int inchannels;
    int outchannels;

    // engine interfaces
    SLObjectItf engineObject;
    SLEngineItf engineEngine;

    // output mix interfaces
    SLObjectItf outputMixObject;
    SLObjectItf playerObject;
    SLAndroidSimpleBufferQueueItf playerBufferQueue;

    // recorder interfaces
    SLObjectItf recorderObject;
    SLAndroidSimpleBufferQueueItf recorderBufferQueue;


} opensles_s;

//Open both audio input and output device
opensles_s* initAudioInOutput(int sr, int inchannels, int outchannels, int bufferframes);
void destroyAudioInOutput(opensles_s *p);

//Open audio input device
opensles_s* initAudioInput(int sr, int inchannels, int bufferframes);
void destroyAudioInput(opensles_s *p);

//Open audio outputput device
opensles_s* initAudioOutput(int sr, int outchannels, int bufferframes);
void destroyAudioOutput(opensles_s *p);

// read buffer
int readInput(opensles_s *p, short *buffer,int size);
int get_AudioInput_bufferSize(opensles_s *p);

// write output buffer
int writeOutput(opensles_s *p, short *buffer,int size);
int writeOutputContinuous(opensles_s *p, short *buffer,int size);
int get_AudioOut_bufferSize(const opensles_s *p);
void setOutputStatus(int value);

#ifdef __cplusplus
};
#endif

#endif // #ifndef OPENSL_BASIC
