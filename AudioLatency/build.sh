#!/bin/sh

# This is the build.sh for the ndk build
# ndk-build build out jni libs in libs directory
# ant debug build out debug apk in bin directory

ndk-build
ant clean
ant debug

