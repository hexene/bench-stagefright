LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=       \
     bench.cpp

LOCAL_LDFLAGS := -L$(HOME)/glib2 -Wl,-rpath,$(HOME)/glib2
LOCAL_LDLIBS := -lstagefright -lmedia -lutils -lbinder

LOCAL_C_INCLUDES:= \
    $(HOME)/android/system/core/include \
    $(HOME)/android/frameworks/base/include \
    $(HOME)/android/frameworks/base/media/libstagefright \
    $(HOME)/android/frameworks/base/include/media/stagefright/openmax 

LOCAL_CFLAGS += -Wno-multichar

LOCAL_MODULE_TAGS := debug

LOCAL_MODULE:= bench

include $(BUILD_EXECUTABLE)
