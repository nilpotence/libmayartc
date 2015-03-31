WEBRTC_TRUNK=/home/$(USER)/libs/webrtc/src

DEFINES=-DWEBRTC_POSIX -DV8_DEPRECATION_WARNINGS -DEXPAT_RELATIVE_PATH \
  -DFEATURE_ENABLE_VOICEMAIL -DGTEST_RELATIVE_PATH -DJSONCPP_RELATIVE_PATH \
  -DLOGGING=1 -DSRTP_RELATIVE_PATH -DFEATURE_ENABLE_SSL -DFEATURE_ENABLE_PSTN \
  -DHAVE_SRTP -DHAVE_WEBRTC_VIDEO -DHAVE_WEBRTC_VOICE -DUSE_WEBRTC_DEV_BRANCH \
  -D_FILE_OFFSET_BITS=64 -DCHROMIUM_BUILD -DUSE_CAIRO=1 -DUSE_GLIB=1 \
  -DUSE_DEFAULT_RENDER_THEME=1 -DUSE_LIBJPEG_TURBO=1 -DUSE_NSS=1 -DUSE_X11=1 \
  -DENABLE_ONE_CLICK_SIGNIN -DGTK_DISABLE_SINGLE_INCLUDES=1 -DUSE_XI2_MT=2 \
  -DENABLE_REMOTING=1 -DENABLE_WEBRTC=1 -DENABLE_PEPPER_CDMS \
  -DENABLE_CONFIGURATION_POLICY -DENABLE_INPUT_SPEECH -DENABLE_NOTIFICATIONS \
  -DUSE_UDEV -DICU_UTIL_DATA_IMPL=ICU_UTIL_DATA_STATIC -DENABLE_EGLIMAGE=1 \
  -DENABLE_TASK_MANAGER=1 -DENABLE_EXTENSIONS=1 \
  -DENABLE_PLUGIN_INSTALLATION=1 -DENABLE_PLUGINS=1 \
  -DENABLE_SESSION_SERVICE=1 -DENABLE_THEMES=1 -DENABLE_BACKGROUND=1 \
  -DENABLE_AUTOMATION=1 -DENABLE_GOOGLE_NOW=1 -DCLD_VERSION=2 \
  -DENABLE_FULL_PRINTING=1 -DENABLE_PRINTING=1 -DENABLE_SPELLCHECK=1 \
  -DENABLE_CAPTIVE_PORTAL_DETECTION=1 -DENABLE_MANAGED_USERS=1 \
  -DENABLE_MDNS=1 -DLIBPEERCONNECTION_LIB=1 -DLINUX -DHAVE_SCTP \
  -DHASH_NAMESPACE=__gnu_cxx -DPOSIX -DDISABLE_DYNAMIC_CAST -D_REENTRANT \
  -DSSL_USE_NSS -DHAVE_NSS_SSL_H -DSSL_USE_NSS_RNG -DNDEBUG -DNVALGRIND \
  -DDYNAMIC_ANNOTATIONS_ENABLED=0

WEBRTC_BIN=${WEBRTC_TRUNK}/out/Release

INCLUDES=-I${WEBRTC_TRUNK} -I${WEBRTC_TRUNK}/third_party \
    -I${WEBRTC_TRUNK}/third_party/webrtc -I${WEBRTC_TRUNK}/webrtc \
    -I${WEBRTC_TRUNK}/net/third_party/nss/ssl \
    -I${WEBRTC_TRUNK}/third_party/jsoncpp/overrides/include \
    -I${WEBRTC_TRUNK}/third_party/jsoncpp/source/include

CFLAGS=-fstack-protector -O3 --param=ssp-buffer-size=4 -pthread \
    -fno-strict-aliasing -Wno-unused-parameter \
    -Wno-missing-field-initializers -pipe -fPIC -pthread -Wno-format \
    -Wno-unused-result -fno-ident -fdata-sections -ffunction-sections \
    -funwind-tables -fno-rtti -fno-threadsafe-statics -Wno-deprecated
    
WEBRTC_LIBS=$(shell find ${WEBRTC_BIN}/ -name "*.a" | grep -v test | grep -v do_not_use)

LIBRARIES=-lgthread-2.0 -lrt -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgio-2.0 \
    -lpangoft2-1.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lcairo -lpango-1.0 \
    -lfreetype -lfontconfig -lgobject-2.0 -lglib-2.0 -lX11 -lXcomposite \
    -lXext -lXrender -lnss3 -lnssutil3 -lsmime3 -lplds4 -lplc4 -lnspr4 -ldl \
    -lexpat -lm

LDFLAGS=-Wl,-z,now -Wl,-z,relro -pthread -Wl,-z,noexecstack -fPIC

CXXFLAGS=${INCLUDES} ${CFLAGS} ${DEFINES} -std=c++11 -MMD -MP

DEBUG=-g

.PHONY: all clean

all: build/Main build/cmain

sigrun:
	node test/server.js
	

run:
	./build/Main

prepare-package:
	cp build/libmayartc.so dist/libmayartc/usr/lib/
	mkdir -p dist/libmayartc/usr/include
	mkdir -p dist/libmayartc/usr/include/mayartc/
	cp src/*.h* dist/libmayartc/usr/include/mayartc/	

build/Main: build/obj/Main.o build/libmayartc.so
	g++ ${DEBUG} ${LDFLAGS} -Lbuild/ build/obj/Main.o -o build/Main -lmayartc
	
build/cmain: build/libmayartc.so src/main.c
	gcc -Lbuild/ -o build/cmain src/main.c -lmayartc

build/libmayartc.a: build/obj/RTCPeer.o build/obj/RTCSignaling.o build/obj/MayaSignaling.o build/obj/RTCConnection.o build/obj/RTCChannel.o
	ar rvs $@ $^ ${WEBRTC_LIBS}

build/libmayartc.so: build/obj/RTCPeer.o build/obj/RTCSignaling.o build/obj/MayaSignaling.o build/obj/RTCConnection.o build/obj/RTCChannel.o build/obj/MemoryVideoCapturer.o build/obj/cwrapper.o
	g++ ${DEBUG} ${LDFLAGS} $^ -Wl,--start-group ${WEBRTC_LIBS} -Wl,--end-group ${LIBRARIES} -shared -o $@

build/obj/%.o: src/%.cpp
	g++ ${DEBUG} ${CXXFLAGS} $< -c -o $@

build/deps/%: src/%.cpp
	g++ ${INCLUDES} ${DEFINES} -H $<
	
-include $(shell find ./build/ -name "*.d")


clean:
	rm -Rf build/*
	mkdir build/obj
	mkdir build/deps
