LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)



LOCAL_SRC_FILES := \
	fb_app.c \



LOCAL_MODULE:= fbdump
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)


