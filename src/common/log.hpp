#pragma once

#ifndef TAG
#define TAG "FOGLTLOGLES"
#endif

#define PRINTF_LOG

#ifdef PRINTF_LOG
    #include <cstdio>
    #define LOGE(...) std::printf( TAG " E: " __VA_ARGS__ ), std::printf("\n")
    #define LOGW(...) std::printf( TAG " W: " __VA_ARGS__ ), std::printf("\n")
    #define LOGI(...) std::printf( TAG " I: " __VA_ARGS__ ), std::printf("\n")
    #define LOGD(...) std::printf( TAG " D: " __VA_ARGS__ ), std::printf("\n")
#else
    #include <android/log.h>
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
    #define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
    #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#endif
