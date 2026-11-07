//
// Created by Daniel on 2026-10-31.
//

#pragma once

#include "usart.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {

#endif

#define LOG_ERR 0
#define LOG_WRN 1
#define LOG_INF 2
#define LOG_DBG 3

bool log_init(UART_HandleTypeDef *huart);
void log_message(char level, const char *tag, const char *fmt, ...);

#ifndef LOG_DEFAULT_LEVEL
#define LOG_DEFAULT_LEVEL LOG_INF
#endif

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL LOG_DEFAULT_LEVEL
#endif

#ifndef LOG_TAG
#define LOG_TAG ""
#endif

#if LOG_LOCAL_LEVEL >= LOG_ERR
#define log_error(...) log_message(LOG_ERR, LOG_TAG, __VA_ARGS__)
#else
#define log_error(...) ((void)0)
#endif

#if LOG_LOCAL_LEVEL >= LOG_WRN
#define log_warning(...) log_message(LOG_WRN, LOG_TAG, __VA_ARGS__)
#else
#define log_warning(...) ((void)0)
#endif

#if LOG_LOCAL_LEVEL >= LOG_INF
#define log_info(...) log_message(LOG_INF, LOG_TAG, __VA_ARGS__)
#else
#define log_info(...) ((void)0)
#endif

#if LOG_LOCAL_LEVEL >= LOG_DBG
#define log_debug(...) log_message(LOG_DBG, LOG_TAG, __VA_ARGS__)
#else
#define log_debug(...) ((void)0)
#endif

#ifdef __cplusplus
}
#endif
