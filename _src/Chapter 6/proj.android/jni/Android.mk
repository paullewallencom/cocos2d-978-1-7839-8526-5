LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := cocos2dcpp_shared

LOCAL_MODULE_FILENAME := libcocos2dcpp

SRCS:= $(shell cd jni;ls -C ../../Classes/*.cpp)
LOCAL_SRC_FILES := hellocpp/main.cpp
LOCAL_SRC_FILES += $(SRCS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Classes

LOCAL_WHOLE_STATIC_LIBRARIES += cocos2dx_static
LOCAL_WHOLE_STATIC_LIBRARIES += cocosdenshion_static
LOCAL_WHOLE_STATIC_LIBRARIES += box2d_static

include $(BUILD_SHARED_LIBRARY)

$(call import-add-path,/opt/cocos2d-x-2.2.5)
$(call import-add-path,/opt/cocos2d-x-2.2.5/cocos2dx/platform/third_party/android/prebuilt)

$(call import-module,cocos2dx)
$(call import-module,CocosDenshion/android)
$(call import-module,external/Box2D)
