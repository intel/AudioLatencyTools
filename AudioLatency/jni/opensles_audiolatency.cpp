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
#include <stdio.h>
#include "opensles.h"
#include "roundtrip/sles.h"
#include "pulseusb_jni.h"
#include "opensles_audiolatency.h"
#include <sys/time.h>
#include <math.h>

#define DUMPTIME 0

static int bufferFrames = 1024;
static int sampleRate = 48000;
static int monoBufferFrames = 256;
static int stereoBufferFrames = 512;

static int record_buffer;
static int running = 0;

static sles_data * pSles = NULL;
void start_roundtrip_google() {
    running = 1;
    pSles = NULL;
    int mMicSource = 3; //maps to MediaRecorder.AudioSource.VOICE_RECOGNITION;
    int mBufferFrames = bufferFrames;
    if (slesInit(&pSles, sampleRate, mBufferFrames, mMicSource) == SLES_FAIL) {
        LOGE("init sles error");
        return;
    }

}
void stop_roundtrip_google() {
    running = 0;
    if (pSles != NULL) {
        slesDestroy(&pSles);
    }
}

void set_audio_parameters(int frames, int rate) {
    if (frames > 0 && rate >0) {
        bufferFrames = frames;
        sampleRate = rate;
        monoBufferFrames = bufferFrames;
        stereoBufferFrames = monoBufferFrames*2; // channel 2
        LOGD("bufferFrames is %d, rate is %d, monoBufferFrames is %d, stereoBufferFrames is %d", bufferFrames,sampleRate,monoBufferFrames,stereoBufferFrames);
    }
}

void start_roundtrip() {
    opensles_s  *p;
    int samps, i, j;
    int mStereo = stereoBufferFrames;
    short * outbuffer = (short *)malloc(sizeof(short) * mStereo);
    if(NULL == outbuffer) {
        LOGE("malloc outbuffer error");
        return;
    }
    memset(outbuffer, 0, sizeof(short)*mStereo);

    int mMono = monoBufferFrames;
    short * inbuffer = (short *)malloc(sizeof(short) * mMono);
    if(NULL == inbuffer) {
        LOGE("malloc inbuffer error");
        free(outbuffer);
        return;
    }
    memset(inbuffer, 0, sizeof(short)* mMono);

    int mBufferFrames = bufferFrames;
    p = initAudioInOutput(sampleRate,1,2,mBufferFrames);
    if(p == NULL) {
        free(inbuffer);
        free(outbuffer);
        return;
    }
    running = 1;
    // write zero buffer for first playback buffer
    for(i = 0, j=0; i < mMono; i++, j+=2)
        outbuffer[j] = outbuffer[j+1] = 0;
    writeOutput(p,outbuffer,mStereo);

    while (running) {
        samps = readInput(p,inbuffer,mMono);
        for (i = 0, j=0; i < samps; i++, j+=2)
            outbuffer[j] = outbuffer[j+1] = inbuffer[i];
        writeOutput(p,outbuffer,samps*2);
    }
    destroyAudioInOutput(p);
    free(inbuffer);
    free(outbuffer);
}

void stop_roundtrip() {
    running = 0;
}

void start_input() {
    opensles_s  *p;
    int samps, i, j;
    FILE *InFile;
    int mMono = monoBufferFrames;
    short * inbuffer = (short *)malloc(sizeof(short) * mMono);
    if (NULL == inbuffer) {
        LOGE("malloc inbuffer error");
        return;
    }
    memset(inbuffer, 0, sizeof(short)*mMono);

    short int * TxIn = (short int *)malloc(sizeof(short int) * mMono);
    if (NULL == TxIn) {
        LOGE("malloc TxIn error");
        free(inbuffer);
        return;
    }

    float sample;
    int flag = 0;

    struct timeval start;
    struct timeval end;

    InFile = fopen("/sdcard/record.pcm", "wb");
    if (InFile == NULL) {
        LOGE("File Operation Failed.");
        free(inbuffer);
        free(TxIn);
        return;
    }

    native_setDtr(1);
    gettimeofday(&start, NULL);
    int mBufferFrames = bufferFrames;
    p = initAudioInput(sampleRate,1,mBufferFrames);
    if(p == NULL) {
        free(inbuffer);
        free(TxIn);
        return;
    }
    running = 1;
    while (running) {
        samps = readInput(p,inbuffer,mMono);
        if( flag == 0 && samps > 0 ) {
            gettimeofday(&end, NULL);
            native_setDtr(0);
            flag = 1;
            LOGD("cold input start: %ld s, %ld us", start.tv_sec, start.tv_usec);
            LOGD("cold input end: %ld s, %ld us", end.tv_sec, end.tv_usec);
        }
        for (i=0; i<samps; i++) {
            sample = (float)inbuffer[i];
            sample = (sample>= 32767?  32767: sample);
            sample = (sample<=-32768? -32768: sample);

            TxIn[i] = (short int)sample;
        }
        fwrite(TxIn, sizeof(short int), samps, InFile);

    }
    destroyAudioInput(p);
    free(inbuffer);
    free(TxIn);
    fclose(InFile);
}

void stop_input() {
    running = 0;
    native_setDtr(0);
}

void start_continuous_input() {
    opensles_s  *p;
    int samps, i, j;
    int mMono = monoBufferFrames;
    short * inbuffer = (short *)malloc(sizeof(short) * mMono);
    if (NULL == inbuffer) {
        LOGE("malloc inbuffer error");
        return;
    }
    memset(inbuffer, 0, sizeof(short)*mMono);
    float sample;
    int interval;
    int noiseInterval = 20;
    float noiseAverage = 0.0;
    float threshold = 0.25f * 32768.0f;

    //showCurrentTime("Start Recording ");
    int mBufferFrames = bufferFrames;
    p = initAudioInput(sampleRate,1,mBufferFrames);
    if (p == NULL) {
        free(inbuffer);
        return;
    }
    running = 1;
    native_setDtr(0);
    int pulseNumber = 0;
    int flag = 0;
    struct timeval start;
    struct timeval end;
    while (running) {
        pulseNumber = 0;
        samps = readInput(p,inbuffer,mMono);
        interval = sampleRate/2000; // sample of 0.5ms
        if (interval >= samps) { //0.5ms
            interval = samps/2;
        }

        if (samps > 0 && flag == 0 ) {
            gettimeofday(&start, NULL);
            pulseNumber = 0;
            // calculate noiseAverage
            if (noiseInterval>0) {
                noiseInterval--;
                float total = 0.0;
                for (i=0; i<samps; i++) {
                    sample = (float)inbuffer[i];
                    total += fabs(sample);
                }
                if ( noiseAverage == 0.0 ) {
                    noiseAverage = total/samps;
                }
                noiseAverage = (noiseAverage + total/samps)/2;
                //LOGD("continuous_input: noiseAverage is %f", noiseAverage );
            } else {
                for (i=0; i<samps; i++) {
                    sample = (float)inbuffer[i];

                    // detect for the pulse pcm data
                    if ( (fabs(sample) > threshold) ) {
                        //LOGD("continuous_input: sample is %f, pulseNum is %d, interval is %d", sample, pulseNumber, interval );
                        pulseNumber++;
                    }

                    if((pulseNumber >= interval) && (flag == 0)) {
                        flag = 1;
                        struct timespec time_sleep;
                        float timeout_us = (1000.0f*1000/sampleRate)*i;
                        gettimeofday(&end, NULL);
                        timeout_us = timeout_us - ((end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec));
                        time_sleep.tv_sec = timeout_us/1000000;
                        time_sleep.tv_nsec = (timeout_us - time_sleep.tv_sec*1000000) * 1000;
                        LOGD("sleep time:tv_sec is %ld, tv_nsec is %ld, samps & detection is %d, %d",time_sleep.tv_sec, time_sleep.tv_nsec, samps, i);
                        if (time_sleep.tv_sec>0 && time_sleep.tv_nsec>0) {
                            nanosleep(&time_sleep,NULL);
                        }
                        native_setDtr(1);
                        break;
                    }
                }

            }

        }

    }
    destroyAudioInput(p);
    free(inbuffer);
}


void stop_continuous_input() {
    running = 0;
    native_setDtr(0);
}

void start_output() {
    opensles_s  *p;
    int samps, i, j;
    short * outbuffer = (short *)malloc(sizeof(short) * stereoBufferFrames);
    if (NULL == outbuffer) {
        LOGE("malloc outbuffer error");
        return;
    }
    memset(outbuffer, 0, sizeof(short)*stereoBufferFrames);

    native_setDtr(1);
    p = initAudioOutput(sampleRate,2,bufferFrames);
    if (p == NULL) {
        LOGE("open audio out device fail");
        free(outbuffer);
        return;
    }
    running = 1;
    //samps = monoBufferFrames;
    samps = get_AudioOut_bufferSize(p);

    setOutputStatus(-1);

    int k = 0;
    int z = 1;
    int interval = 48;
    interval = sampleRate/1000;
    if (interval < 0) {
        interval = 48; // 48000
    }
    float amplitude = 32767.0;
    float gain = 0.866; //-3db
    amplitude = amplitude * gain;

    while (running) {
        for (i = 0; i < samps; i++) {
            if (k == interval) {
                z = -1*z;
                k = 0;
            }

            //outbuffer[i] = z*32767 16883;
            outbuffer[i] = z*amplitude;
            k++;
        }

        writeOutput(p,outbuffer,samps);
    }
    destroyAudioOutput(p);
    free(outbuffer);
}
void stop_output() {
    running = 0;
    setOutputStatus(-1);
    native_setDtr(0);
}

void start_continuous_output() {
    opensles_s  *p;
    int samps, i, j;
    short * outbuffer = (short *)malloc(sizeof(short) * stereoBufferFrames);
    if (NULL == outbuffer) {
        LOGE("malloc outbuffer error");
        return;
    }
    memset(outbuffer, 0, sizeof(short)*stereoBufferFrames);


    int pcmValue = 0; // 0 or 9
    int signalValue = 0;
    struct timeval startTime;
    struct timeval endTime;
    int timeInterval = 3;


    native_setDtr(0);

    p = initAudioOutput(sampleRate,2,bufferFrames);
    if (p == NULL) {
        LOGE("open audio out device fail");
        free(outbuffer);
        return;
    }
    running = 1;
    samps = get_AudioOut_bufferSize(p);

    // wait for 10s to play 0 data
    gettimeofday( &startTime, NULL );
    int times = 0;

    setOutputStatus(-1);
    // start the continuous out test

    int k = 0;
    int z = 1;
    int interval = 48;
    interval = sampleRate/1000;
    if (interval < 0) {
        interval = 48; // 48000
    }

    float amplitude = 32767.0;
    float gain = 0.866; //-3db
    amplitude = amplitude * gain;

    while (running) {
        for (i = 0; i < samps; i++) {
            if (pcmValue < 0.5) {
                outbuffer[i] = 0;
            } else {
                if (k == interval) {
                    z = -1*z;
                    k = 0;
                }

                outbuffer[i] = z*amplitude;
                k++;
            }
        }
        writeOutputContinuous(p,outbuffer,samps);
        times++;
        if (times >= 20) {
            gettimeofday(&endTime, NULL);
            times = 0;
            if (endTime.tv_sec - startTime.tv_sec >= timeInterval) {
                startTime = endTime;
                pcmValue = (pcmValue==0)?1:0;
                signalValue = (signalValue==0)?1:0;
                setOutputStatus(signalValue);
            }
        }

    }

    destroyAudioOutput(p);
    free(outbuffer);
}

void stop_continuous_output() {
    running = 0;
    setOutputStatus(-1);
    native_setDtr(0);
}

void showCurrentTime(char * message) {
    if(DUMPTIME) {
        struct timeval start;
        gettimeofday( &start, NULL );
        LOGD("%s time is %ld s : %ld us\n", message, start.tv_sec , start.tv_usec);
    }
}

void native_setDtr_sles(int value) {
    native_setDtr(value);
}

int signal_symbol = 0;
int signal_flag = 0;

void changeDtrStatus(int num) {
    native_setDtr(signal_flag);
    signal_flag = (signal_flag == 0 ) ? 1 : 0;
}
void start_usb2gpio_calibration() {
    signal(SIGALRM, changeDtrStatus);
    struct itimerval value;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 5000; // 5ms
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 5000;
    setitimer(ITIMER_REAL, &value, NULL);
    start_output();
}

void stop_usb2gpio_calibration() {
    stop_output();
    native_setDtr(0);
    struct itimerval value;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = 0; // 0ms
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &value, NULL);
}
