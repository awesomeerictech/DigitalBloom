#!/bin/sh

ANDROID_NDK=/c/androidsdk/ndk/29.0.14206865
PATH=$PATH:$ANDROID_NDK
export PATH
API_LEVEL=23
mkdir opensslandroid
mkdir build
BUILD_DIR=openssl/build
#Debian and Kali distributions
OUT_DIR=/c/Qt/dev/DigitalBloom/openssl/opensslandroid
#Ubuntu Linux
#OUT_DIR=/home/programmereric/openssl/opensslandroid
BUILD_TARGETS="armeabi-v7a arm64-v8a x86 x86_64"
#BUILD_TARGETS="armeabi-v7a"
ASM_OPTIONS=""
TOOLCHAIN=$ANDROID_NDK/toolchains/llvm/prebuilt/windows-x86_64



unzip openssl.zip
cd openssl || exit 128


##### Prepare Files #####

sed -i 's/.*-mandroid.*//' Configurations/15-android.conf
patch -p1 -N <<EOP
--- old/Configurations/unix-Makefile.tmpl   2018-09-11 14:48:19.000000000 +0200
+++ new/Configurations/unix-Makefile.tmpl   2018-10-18 09:06:27.282007245 +0200
@@ -43,12 +43,17 @@
      # will return the name from shlib(\$libname) with any SO version number
      # removed.  On some systems, they may therefore return the exact same
      # string.
-     sub shlib {
+     sub shlib_simple {
          my \$lib = shift;
          return () if \$disabled{shared} || \$lib =~ /\\.a$/;
-         return \$unified_info{sharednames}->{\$lib}. \$shlibvariant. '\$(SHLIB_EXT)';
+
+         if (windowsdll()) {
+             return \$lib . '\$(SHLIB_EXT_IMPORT)';
+         }
+         return \$lib .  '\$(SHLIB_EXT_SIMPLE)';
      }
-     sub shlib_simple {
+     
+   sub shlib {
          my \$lib = shift;
          return () if \$disabled{shared} || \$lib =~ /\\.a$/;

EOP

##### remove output-directory #####
rm -rf $OUT_DIR

##### export ndk directory. Required by openssl-build-scripts #####
export ANDROID_NDK

##### build-function #####
build_the_thing() {
    export PATH=$TOOLCHAIN/$TRIBLE/bin:$TOOLCHAIN/bin:"$PATH"
echo $PATH
    make clean
    export CC=${MYCROSS_COMPILECC}
    export CXX=${MYCROSS_COMPILEXX}
    export CROSS_COMPILE=""
    export ANDROID_SYSROOT=$TOOLCHAIN/sysroot
    export AR=$TOOLCHAIN/bin/llvm-ar
    export LD=$TOOLCHAIN/bin/lld
    export RANLIB=$TOOLCHAIN/bin/llvm-ranlib
    export STRIP=$TOOLCHAIN/bin/llvm-strip
    export NM=$TOOLCHAIN/bin/llvm-nm
    export CROSS_SYSROOT=$ANDROID_SYSROOT
    ./Configure  $OPTIONS $ASM_OPTIONS no-engine no-dso no-tests   && \
    make && \
    make install DESTDIR=$DESTDIR || exit 128
   #
}

##### set variables according to build-tagret #####
for build_target in $BUILD_TARGETS
do
    
case $build_target in
    armeabi-v7a)
        TRIBLE="arm-linux-androideabi"
        MYCROSS_COMPILECC="armv7a-linux-androideabi${API_LEVEL}-clang"
        MYCROSS_COMPILEXX="armv7a-linux-androideabi${API_LEVEL}-clang++"
        ASM_OPTIONS="linux-generic32"
        TC_NAME="arm-linux-androideabi-4.9"
        OPTIONS="zlib -latomic -DANDROID_NDK -Wl,--fix-cortex-a8 -fPIC"
        DESTDIR="/home/ericiots/eric/$BUILD_DIR/armeabi-v7a"
        
    ;;
    x86)
        TRIBLE="i686-linux-android"
        MYCROSS_COMPILECC="i686-linux-android${API_LEVEL}-clang"
        MYCROSS_COMPILEXX="i686-linux-android${API_LEVEL}-clang++"
        ASM_OPTIONS="linux-generic32 386"
        TC_NAME="x86-4.9"
        OPTIONS="zlib -latomic -fPIC "
        DESTDIR="/home/ericiots/eric/$BUILD_DIR/x86"
       
    ;;
    x86_64)
        TRIBLE="x86_64-linux-android"
        MYCROSS_COMPILECC="x86_64-linux-android${API_LEVEL}-clang"
        MYCROSS_COMPILEXX="x86_64-linux-android${API_LEVEL}-clang++"
        ASM_OPTIONS="linux-generic64 enable-ec_nistp_64_gcc_128"
        TC_NAME="x86_64-4.9"
        OPTIONS="zlib -latomic -fPIC "
        DESTDIR="/home/ericiots/eric/$BUILD_DIR/x86_64"
        
    ;;
    arm64-v8a)
        TRIBLE="aarch64-linux-android"
        MYCROSS_COMPILECC="aarch64-linux-android${API_LEVEL}-clang"
        MYCROSS_COMPILEXX="aarch64-linux-android${API_LEVEL}-clang++"
        ASM_OPTIONS="linux-generic64 enable-ec_nistp_64_gcc_128"
        TC_NAME="aarch64-linux-android-4.9"
        OPTIONS="zlib -latomic -fPIC "
        DESTDIR="/home/ericiots/eric/$BUILD_DIR/arm64-v8a"
        
    ;;
    esac

    rm -rf $DESTDIR
    build_the_thing
#### copy libraries and includes to output-directory #####
    mkdir -p $OUT_DIR/include/$build_target
    cp -R $DESTDIR/usr/local/include/* $OUT_DIR/include/$build_target
    mkdir -p $OUT_DIR/lib/$build_target
    cp -R $DESTDIR/usr/local/lib/*.so $OUT_DIR/lib/$build_target
done

echo Success
rm -rf openssl
