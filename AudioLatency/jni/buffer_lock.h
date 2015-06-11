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


#ifndef BUFFER_LOCK_OPENSL
#define BUFFER_LOCK_OPENSL
#include <pthread.h>
#include <stdlib.h>

#include<android/log.h>
#define LOG_TAG "OpenSL-AudioLatency"
#define LOGV(fmt, args...) __android_log_print(ANDROID_LOG_VERBOSE,  LOG_TAG, fmt, ##args)
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args)

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    unsigned int   symbol;
} bufferLock;

void* createBufferLock(void);
void waitBufferLock(void *lock);
void signalBufferLock(void *lock);
void destroyBufferLock(void *lock);

#endif
