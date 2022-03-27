#pragma once

#ifdef _WIN32
    #include <stdio.h>
    #define LOGE(TAG, ...) printf("Error   in file %s, at Line: %d", __FILE__, __LINE__); printf(TAG); printf(__VA_ARGS__); printf("\n")
    #define LOGW(TAG, ...) printf("Warning in file %s, at Line: %d", __FILE__, __LINE__); printf(TAG); printf(__VA_ARGS__); printf("\n")
    #define LOGD(TAG, ...) printf("Info    in file %s, at Line: %d", __FILE__, __LINE__); printf(TAG); printf(__VA_ARGS__); printf("\n")
#else
    #include <android/log.h>
    #define LOGE(TAG, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
    #define LOGW(TAG, ...) __android_log_print(ANDROID_LOG_WA, TAG, __VA_ARGS__)
    #define LOGD(TAG, ...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#endif