# this is the README for android-audiolatencytest

5 cases are supported:
1. roundtrip
2. cold output
3. cold input
4. continuous output
5. continuous input

# before start to build, please confirm if you have Android SDK and NDK in your machine.
Build Instruction
This project can be built by only command line with steps as following:
1. Preparation for the enviroment (ubuntu as example)

  a) download android ndk
      https://developer.android.com/ndk/downloads/index.html#Downloads

  b) download android sdk
      https://developer.android.com/tools/sdk/tools-notes.html

  c) export ndk and sdk to PATH in ~/.bashrc
      export ANDROID_NDK=/home/yijin/tools/Android_tools/android-ndk-r10d
      PATH=$ANDROID_NDK:$PATH
      export ANDROID_SDK=/home/yijin/tools/Android_tools/android-sdk-linux/tools/
      PATH=$ANDROID_SDK:$PATH
      export ANDROID_HOME=/home/yijin/tools/Android_tools/android-sdk-linux/
      PATH=$ANDROID_HOME:$PATH

  d) install android sdk, built tools and platform tools
      https://developer.android.com/sdk/installing/adding-packages.html

  e) install ant
      sudo apt-get install ant


2. Build project
  #./build.sh   // ndk-build & ant debug

Tips for build problems:
1. Unable to resolve target 'android-**'
    a) use command "android list targets" to get your sdk target version
    b) use the current target version in file project.properties
2.  Cannot run program "**/${aapt}": error=2, No such file or directory
    a) use your build tools path in build.xml   : <property name="android.build.tools.dir" location="${sdk.dir}/build-tools/22.0.0" />


You can also use ndk-build to built the native libs and then use eclipse to build the apk.
