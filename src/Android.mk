LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	jni/SDL_image \
	jni/SDL_mixer \
	jni/SDL_gfx \
	jni/SDL/include \
	jni/SDL \
	jni/SDL_ttf

# Add any compilation flags for your project here...
LOCAL_CFLAGS := \
	-std=c++11
#	-DDEV_BUILD

FILE_LIST := $(notdir $(wildcard $(LOCAL_PATH)/*.cpp))

LOCAL_SRC_FILES := $(FILE_LIST)

LOCAL_SHARED_LIBRARIES := SDL SDL_image SDL_mixer SDL_ttf SDL_gfx

LOCAL_LDLIBS := -lGLESv1_CM -llog

include $(BUILD_SHARED_LIBRARY)
