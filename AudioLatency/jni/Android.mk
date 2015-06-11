LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libpulseusb
LOCAL_SRC_FILES := pulseusb_jni.cpp 
LOCAL_C_INCLUDES:= \
        $(call include-path-for, wilhelm) \
        $(LOCAL_PATH) \

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE   := libaudiolatency_opensles
LOCAL_C_INCLUDES:= \
        $(call include-path-for, wilhelm) \
        $(LOCAL_PATH) \
        $(LOCAL_PATH)/google \
LOCAL_CFLAGS := -O3 -DSTDC_HEADERS
LOCAL_CPPFLAGS :=$(LOCAL_CFLAGS)
LOCAL_LDFLAGS := -Wl,--hash-style=sysv

###

LOCAL_SRC_FILES := opensles_audiolatency.cpp \
        opensles.c \
        android_media_opensles_audiolatency_java_api.cpp \
        buffer_lock.c \
        google/roundtrip/sles.cpp \
        google/audio_utils/atomic.c \
        google/audio_utils/fifo.c \
        google/audio_utils/roundup.c

LOCAL_SHARED_LIBRARIES := \
        libutils \
        libcutils \
        libpulseusb \
        libstlport

LOCAL_LDLIBS := -llog \
        -lOpenSLES \
        

include $(BUILD_SHARED_LIBRARY)


