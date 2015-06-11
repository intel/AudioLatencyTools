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


#ifndef OPENSLES_AUDIOLATENCY
#define OPENSLES_AUDIOLATENCY
#ifdef __cplusplus
extern "C" {
#endif
void start_roundtrip();
void stop_roundtrip();
void start_roundtrip_google();
void stop_roundtrip_google();
void start_input();
void stop_input();
void start_output();
void stop_output();
void start_continuous_output();
void stop_continuous_output();
void start_continuous_input();
void stop_continuous_input();
void showCurrentTime(char * message);
void native_setDtr_sles(int value);
void set_audio_parameters(int frames, int rate );
void start_usb2gpio_calibration();
void stop_usb2gpio_calibration();
#ifdef __cplusplus
};
#endif
#endif

