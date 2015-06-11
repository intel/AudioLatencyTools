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

#include "buffer_lock.h"

//----------------------------------------------------------------------
// buffer Locks
void* createBufferLock(void)
{
    bufferLock  *pLock;
    pLock = (bufferLock*) malloc(sizeof(bufferLock));
    if (pLock == NULL)
        return NULL;
    memset(pLock, 0, sizeof(bufferLock));

    if (pthread_mutex_init(&(pLock->mutex), NULL) != 0) {
        pthread_mutex_destroy(&(pLock->mutex));
        free((void*) pLock);
        return NULL;
    }
    if (pthread_cond_init(&(pLock->cond), NULL) != 0) {
        pthread_mutex_destroy(&(pLock->mutex));
        free((void*) pLock);
        return NULL;
    }
    pLock->symbol = 0;

    return pLock;
}

void waitBufferLock(void *lock)
{
    bufferLock  *pLock = (bufferLock*) lock;
    pthread_mutex_lock(&(pLock->mutex));
    while (!pLock->symbol) {
        pthread_cond_wait(&(pLock->cond), &(pLock->mutex));
    }
    pLock->symbol = 0;
    pthread_mutex_unlock(&(pLock->mutex));
}

void signalBufferLock(void *lock)
{
    bufferLock *pLock = (bufferLock*) lock;
    pthread_mutex_lock(&(pLock->mutex));
    pLock->symbol = 1;
    pthread_cond_signal(&(pLock->cond));
    pthread_mutex_unlock(&(pLock->mutex));
}

void destroyBufferLock(void *lock)
{
    bufferLock  *pLock = (bufferLock*) lock;
    if (pLock == NULL)
        return;
    signalBufferLock(pLock);
    pthread_cond_destroy(&(pLock->cond));
    pthread_mutex_destroy(&(pLock->mutex));
    free(pLock);
}
