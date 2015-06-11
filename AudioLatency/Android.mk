#
#  Use of this source code is governed by a BSD-style license
#  that can be found in the LICENSE file in the root of the source
#  tree. An additional intellectual property rights grant can be found
#  in the file PATENTS.  All contributing project authors may
#  be found in the AUTHORS file in the root of the source tree.

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := tests

LOCAL_SRC_FILES := \
    src/com/intel/android_audio_latency/AudiolatencytestActivity.java \
    src/com/intel/android_audio_latency/audiolatency_openslJNI.java \
    src/com/intel/android_audio_latency/PulseUSB.java

LOCAL_PACKAGE_NAME := audio-latency
LOCAL_CERTIFICATE := platform

LOCAL_JNI_SHARED_LIBRARIES := libopensles_audiolatency libpulseusb
LOCAL_LDLIBS := -llog

include $(BUILD_PACKAGE)
include $(call all-makefiles-under,$(LOCAL_PATH))
