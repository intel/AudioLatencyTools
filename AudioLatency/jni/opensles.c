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

#include "opensles.h"
#include "pulseusb_jni.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ASSERT_EQ(x, y) do { if ((x) == (y)) ; else { fprintf(stderr, "0x%x != 0x%x\n", \
    (unsigned) (x), (unsigned) (y)); assert((x) == (y)); } } while (0)
#define NB_BUFFERS_IN_QUEUE 1

static void recorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
static void playerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

static int record_get_data = 0;
static int output_status = -1;


static SLuint32 getSR(SLuint32 sr) {
    SLuint32 sampleRate = -1;
    switch(sr) {

    case 8000:
        sampleRate = SL_SAMPLINGRATE_8;
        break;
    case 11025:
        sampleRate = SL_SAMPLINGRATE_11_025;
        break;
    case 16000:
        sampleRate = SL_SAMPLINGRATE_16;
        break;
    case 22050:
        sampleRate = SL_SAMPLINGRATE_22_05;
        break;
    case 24000:
        sampleRate = SL_SAMPLINGRATE_24;
        break;
    case 32000:
        sampleRate = SL_SAMPLINGRATE_32;
        break;
    case 44100:
        sampleRate = SL_SAMPLINGRATE_44_1;
        break;
    case 48000:
        sampleRate = SL_SAMPLINGRATE_48;
        break;
    case 64000:
        sampleRate = SL_SAMPLINGRATE_64;
        break;
    case 88200:
        sampleRate = SL_SAMPLINGRATE_88_2;
        break;
    case 96000:
        sampleRate = SL_SAMPLINGRATE_96;
        break;
    case 192000:
        sampleRate = SL_SAMPLINGRATE_192;
        break;
    default:
        sampleRate = -1;
    }
    return sampleRate;

}



// creates the OpenSL ES audio engine
static SLresult openSLInitEngine(opensles_s *p)
{
    SLresult result;
    // create engine
    result = slCreateEngine(&(p->engineObject), 0, NULL, 0, NULL, NULL);
    ASSERT_EQ(SL_RESULT_SUCCESS, result);

    // realize the engine
    result = (*p->engineObject)->Realize(p->engineObject, SL_BOOLEAN_FALSE);
    ASSERT_EQ(SL_RESULT_SUCCESS, result);

    result = (*p->engineObject)->GetInterface(p->engineObject, SL_IID_ENGINE, &(p->engineEngine));
    ASSERT_EQ(SL_RESULT_SUCCESS, result);

    return result;
}

// opens the OpenSL ES device for output
static SLresult openSLInitOutput(opensles_s *p, int sampleRate, int outchannels, int bufferframes)
{
    SLresult result;

    p->sampleRate = sampleRate;
    p->outchannels = outchannels;
    p->outBufferLock = createBufferLock();

    if ((p->outBufSamples  =  bufferframes*outchannels) != 0) {
        p->outputBuffer[0] = (short *) calloc(p->outBufSamples, sizeof(short));
        p->outputBuffer[1] = (short *) calloc(p->outBufSamples, sizeof(short));
        if ((p->outputBuffer[0] == NULL) || (p->outputBuffer[1] == NULL)) {
            if (p->outputBuffer[0] != NULL)
                free(p->outputBuffer[0]);
            if (p->outputBuffer[1] != NULL)
                free(p->outputBuffer[1]);
            return -1;
        }
    }

    p->currentOutPositionIndex = 0;
    p->currentOutBufferIndex  = 0;


    if(p->outchannels) {
        // configure audio source
        SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, NB_BUFFERS_IN_QUEUE};

        SLuint32 sr = getSR(p->sampleRate);
        if (sr == -1) {
            LOGE("The input sr is invalid");
            return -1;
        }

        result = (*p->engineEngine)->CreateOutputMix(p->engineEngine, &(p->outputMixObject), 0, NULL, NULL);
        if (result != SL_RESULT_SUCCESS) return result;

        // realize the output mix
        result = (*p->outputMixObject)->Realize(p->outputMixObject, SL_BOOLEAN_FALSE);

        int channelMask = (p->outchannels == 1) ? SL_SPEAKER_FRONT_CENTER :
                (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT);

        SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM,p->outchannels, sr,
                                       SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                       channelMask, SL_BYTEORDER_LITTLEENDIAN
                                      };
        SLDataSource audioSrc = {&loc_bufq, &format_pcm};

        // configure audio sink
        SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, p->outputMixObject};
        SLDataSink audioSnk = {&loc_outmix, NULL};

        // create audio player
        const SLInterfaceID ids[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
        const SLboolean req[] = {SL_BOOLEAN_TRUE};
        result = (*p->engineEngine)->CreateAudioPlayer(p->engineEngine, &(p->playerObject), &audioSrc, &audioSnk,
                 1, ids, req);
        ASSERT_EQ(SL_RESULT_SUCCESS, result);

        // realize the player
        result = (*p->playerObject)->Realize(p->playerObject, SL_BOOLEAN_FALSE);
        ASSERT_EQ(SL_RESULT_SUCCESS, result);

        // get the play interface
        SLPlayItf playerPlay;
        result = (*p->playerObject)->GetInterface(p->playerObject, SL_IID_PLAY, &playerPlay);
        ASSERT_EQ(SL_RESULT_SUCCESS, result);

        // get the buffer queue interface
        result = (*p->playerObject)->GetInterface(p->playerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                 &(p->playerBufferQueue));
        ASSERT_EQ(SL_RESULT_SUCCESS, result);

        // register callback on the buffer queue
        result = (*p->playerBufferQueue)->RegisterCallback(p->playerBufferQueue, playerCallback, p);
        ASSERT_EQ(SL_RESULT_SUCCESS, result);

        // set the player's state to playing
        result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
        ASSERT_EQ(SL_RESULT_SUCCESS, result);

        return result;
    }
    return -1;
}

// Open the OpenSL ES device for input
static SLresult openSLInitInput(opensles_s *p, int sampleRate, int inchannels, int bufferframes) {

    SLresult result;

    p->inchannels = inchannels;
    p->sampleRate = sampleRate;
    p->inBufferLock = createBufferLock();

    if ((p->inBufSamples  =  bufferframes*inchannels) != 0) {
        p->inputBuffer[0] = (short *) calloc(p->inBufSamples, sizeof(short));
        p->inputBuffer[1] = (short *) calloc(p->inBufSamples, sizeof(short));
        if((p->inputBuffer[0] == NULL) || (p->inputBuffer[1] == NULL)) {
            if (p->inputBuffer[0] != NULL)
                free(p->inputBuffer[0]);
            if (p->inputBuffer[1] != NULL)
                free(p->inputBuffer[1]);
            return -1;
        }
    }

    p->currentInPositionIndex = p->inBufSamples;
    p->currentInBufferIndex = 0;

    if (p->inchannels) {

        SLuint32 sr = getSR(p->sampleRate);
        if (sr == -1) {
            LOGE("The sr is invlaid");
            return -1;
        }

        // configure audio source
        SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
                                          SL_DEFAULTDEVICEID_AUDIOINPUT, NULL
                                         };
        SLDataSource audioSrc = {&loc_dev, NULL};

        // configure audio sink
        int channelMask = (p->inchannels == 1) ? SL_SPEAKER_FRONT_CENTER :
                (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT);

        SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, NB_BUFFERS_IN_QUEUE};
        SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, p->inchannels, sr,
                                       SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                       channelMask, SL_BYTEORDER_LITTLEENDIAN
                                      };
        SLDataSink audioSnk = {&loc_bq, &format_pcm};

        // create audio recorder
        const SLInterfaceID id[2] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_ANDROIDCONFIGURATION};
        const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_FALSE};
        result = (*p->engineEngine)->CreateAudioRecorder(p->engineEngine, &(p->recorderObject), &audioSrc,
                 &audioSnk, 2, id, req);
        ASSERT_EQ(SL_RESULT_SUCCESS, result);

        // realize the audio recorder
        result = (*p->recorderObject)->Realize(p->recorderObject, SL_BOOLEAN_FALSE);
        ASSERT_EQ(SL_RESULT_SUCCESS, result);

        // get the record interface
        SLRecordItf recorderRecord;
        result = (*p->recorderObject)->GetInterface(p->recorderObject, SL_IID_RECORD, &(recorderRecord));
        ASSERT_EQ(SL_RESULT_SUCCESS, result);

        // get the buffer queue interface
        result = (*p->recorderObject)->GetInterface(p->recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                 &(p->recorderBufferQueue));
        ASSERT_EQ(SL_RESULT_SUCCESS, result);

        // register callback on the buffer queue
        result = (*p->recorderBufferQueue)->RegisterCallback(p->recorderBufferQueue, recorderCallback,
                 p);
        ASSERT_EQ(SL_RESULT_SUCCESS, result);
        result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_RECORDING);

        ASSERT_EQ(SL_RESULT_SUCCESS, result);
        return result;
    }
    else return -1;


}


// open the android audio device for input and/or output
opensles_s *initAudioInOutput(int sr, int inchannels, int outchannels, int bufferframes) {

    opensles_s *p;
    p = (opensles_s *) calloc(sizeof(opensles_s),1);
    if (p==NULL) {
        LOGE("calloc opensles failure");
        return NULL;
    }

    if (openSLInitEngine(p) != SL_RESULT_SUCCESS) {
        destroyAudioInOutput(p);
        return NULL;
    }

    if (openSLInitInput(p, sr, inchannels, bufferframes) != SL_RESULT_SUCCESS) {
        destroyAudioInOutput(p);
        return NULL;
    }

    if(openSLInitOutput(p, sr, outchannels, bufferframes) != SL_RESULT_SUCCESS) {
        destroyAudioInOutput(p);
        return NULL;
    }

    record_get_data = 0;
    signalBufferLock(p->outBufferLock);
    signalBufferLock(p->inBufferLock);

    return p;
}

// open the android audio device for input
opensles_s *initAudioInput(int sr, int inchannels, int bufferframes) {

    opensles_s *p;
    p = (opensles_s *) calloc(sizeof(opensles_s),1);
    if (p==NULL) {
        LOGE("calloc opensles failure");
        return NULL;
    }

    if(openSLInitEngine(p) != SL_RESULT_SUCCESS) {
        destroyAudioInput(p);
        return NULL;
    }

    if(openSLInitInput(p, sr, inchannels, bufferframes) != SL_RESULT_SUCCESS) {
        destroyAudioInput(p);
        return NULL;
    }

    record_get_data = 0;
    signalBufferLock(p->inBufferLock);

    return p;
}

// open the android audio device for output
opensles_s *initAudioOutput(int sr, int outchannels, int bufferframes) {

    opensles_s *p;
    p = (opensles_s *) calloc(sizeof(opensles_s),1);
    if (p==NULL) {
        LOGE("calloc opensles failure");
        return NULL;
    }

    if (openSLInitEngine(p) != SL_RESULT_SUCCESS) {
        destroyAudioOutput(p);
        return NULL;
    }

    if (openSLInitOutput(p, sr, outchannels, bufferframes) != SL_RESULT_SUCCESS) {
        destroyAudioOutput(p);
        return NULL;
    }

    signalBufferLock(p->outBufferLock);

    return p;
}

void clearAudioInBuffer(opensles_s *p) {
    record_get_data = 0;
    if (p->inBufferLock != NULL) {
        signalBufferLock(p->inBufferLock);
        destroyBufferLock(p->inBufferLock);
        p->inBufferLock = NULL;
    }

    if (p->inputBuffer[0] != NULL)
        free(p->inputBuffer[0]);

    if (p->inputBuffer[1] != NULL)
        free(p->inputBuffer[1]);

}
void clearAudioOutBuffer(opensles_s *p) {
    if (p->outBufferLock != NULL) {
        signalBufferLock(p->outBufferLock);
        destroyBufferLock(p->outBufferLock);
        p->inBufferLock = NULL;
    }

    if (p->outputBuffer[0] != NULL)
        free(p->outputBuffer[0]);

    if (p->outputBuffer[1] != NULL)
        free(p->outputBuffer[1]);

}
static void openSLDestroyOutput(opensles_s *p) {

    if (p->playerObject != NULL) {
        SLPlayItf playerPlay;
        SLresult result = (*(p->playerObject))->GetInterface(p->playerObject, SL_IID_PLAY,
                          &playerPlay);

        ASSERT_EQ(SL_RESULT_SUCCESS, result);

        //stop player and recorder if they exist
        result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
        ASSERT_EQ(SL_RESULT_SUCCESS, result);
        (*p->playerObject)->Destroy(p->playerObject);
        p->playerObject = NULL;
        playerPlay = NULL;
        p->playerBufferQueue = NULL;
    }

    if (p->outputMixObject != NULL) {
        (*p->outputMixObject)->Destroy(p->outputMixObject);
        p->outputMixObject = NULL;
    }

    clearAudioOutBuffer(p);
}


static void openSLDestroyInput(opensles_s *p) {

    if (p->recorderObject != NULL) {
        SLRecordItf recorderRecord;
        SLresult result = (*(p->recorderObject))->GetInterface(p->recorderObject, SL_IID_RECORD,
                          &recorderRecord);
        ASSERT_EQ(SL_RESULT_SUCCESS, result);


        result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
        ASSERT_EQ(SL_RESULT_SUCCESS, result);

        (*p->recorderObject)->Destroy(p->recorderObject);
        p->recorderObject = NULL;
        recorderRecord = NULL;
        p->recorderBufferQueue = NULL;
    }

    clearAudioInBuffer(p);
}


static void openSLDestroyEngine(opensles_s *p) {

    if (p->engineObject != NULL) {
        (*p->engineObject)->Destroy(p->engineObject);
        p->engineObject = NULL;
        p->engineEngine = NULL;
    }

}


// close the android audio device
void destroyAudioInOutput(opensles_s *p) {

    if (p == NULL)
        return;

    openSLDestroyInput(p);
    openSLDestroyOutput(p);
    openSLDestroyEngine(p);

    free(p);
}

// close the android audio device
void destroyAudioInput(opensles_s *p) {

    if (p == NULL)
        return;

    openSLDestroyInput(p);
    openSLDestroyEngine(p);

    free(p);
}

// close the android audio out device
void destroyAudioOutput(opensles_s *p) {

    if (p == NULL)
        return;

    openSLDestroyOutput(p);
    openSLDestroyEngine(p);

    free(p);
}


// Called after audio recorder fills a buffer with data
void recorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    opensles_s *p = (opensles_s *) context;
    if (p != NULL) {
        signalBufferLock(p->inBufferLock);
        if (record_get_data == 0 ) {
            struct timeval start;
            gettimeofday(&start, NULL);
            LOGD("get first record data in callback %ld s : %ld us",start.tv_sec, start.tv_usec);
        }
        record_get_data = 1;
    }
}


int get_AudioInput_bufferSize(opensles_s *p) {
    return p->inBufSamples;
}

// gets a buffer of size samples from the device
int readInput(opensles_s *p,short *buffer,int size) {
    if (p == NULL) {
        LOGE("opensles struct is NULL");
        return 0;
    }

    int i = 0;
    int index = p->currentInPositionIndex;
    short * inBuffer = p->inputBuffer[p->currentInBufferIndex];
    if (p->inBufSamples == 0 || inBuffer == NULL)  return 0;


    for (i=0; i < size; i++) {
        if (index >= p->inBufSamples) {
            waitBufferLock(p->inBufferLock);
            (*p->recorderBufferQueue)->Enqueue(p->recorderBufferQueue,
                                               inBuffer,p->inBufSamples*sizeof(short));
            p->currentInBufferIndex = (p->currentInBufferIndex ? 0 : 1);
            index = 0;
            inBuffer = p->inputBuffer[p->currentInBufferIndex];
        }
        buffer[i] = (float) inBuffer[index++];
    }
    p->currentInPositionIndex = index;
    if(record_get_data != 0) {
        return i;
    }
    return 0;
}

// Called after audio player empties a buffer of data
void playerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    opensles_s *p = (opensles_s *) context;
    if( p != NULL) {
        signalBufferLock(p->outBufferLock);
    }
}



// set the output_status when change the data
void setOutputStatus(int value) {
    output_status = value;
}

int get_AudioOut_bufferSize(const opensles_s *p) {
    return p->outBufSamples;
}
// puts a buffer of size samples to the device
int writeOutput(opensles_s *p, short *buffer,int size) {
    if (p == NULL) {
        LOGE("opensles struct is NULL");
        return 0;
    }

    int i = 0;
    int index = p->currentOutPositionIndex;
    short * outBuffer = p->outputBuffer[p->currentOutBufferIndex];
    if (p->outBufSamples == 0 || outBuffer == NULL)  return 0;

    for (i=0; i < size; i++) {
        outBuffer[index++] = (short) (buffer[i]);
        if (index >= p->outBufSamples) {
            waitBufferLock(p->outBufferLock);

            (*p->playerBufferQueue)->Enqueue(p->playerBufferQueue,
                                             outBuffer,p->outBufSamples*sizeof(short));
            p->currentOutBufferIndex = (p->currentOutBufferIndex ?  0 : 1);
            index = 0;
            outBuffer = p->outputBuffer[p->currentOutBufferIndex];
        }
    }
    p->currentOutPositionIndex = index;
    return i;
}

// puts a buffer of size samples to the device
int writeOutputContinuous(opensles_s *p, short *buffer,int size) {
    if (p == NULL) {
        LOGE("opensles struct is NULL");
        return 0;
    }

    int i = 0;
    int index = p->currentOutPositionIndex;
    short * outBuffer = p->outputBuffer[0];
    if (p->outBufSamples == 0 || outBuffer == NULL)  return 0;


    for (i=0; i < size; i++) {
        outBuffer[index++] = (short) (buffer[i]);
        if (index >= p->outBufSamples) {

            if(output_status == 0) { // pcm value change to 0
                native_setDtr_sles(0);
                output_status = -1;
            } else if(output_status == 1) { // pcm value change to 1
                native_setDtr_sles(1);
                output_status = -1;
            }
            (*p->playerBufferQueue)->Enqueue(p->playerBufferQueue,
                                             outBuffer,p->outBufSamples*sizeof(short));
            waitBufferLock(p->outBufferLock);

            index = 0;
        }
    }
    p->currentOutPositionIndex = index;
    return i;
}
