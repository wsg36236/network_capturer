#pragma once

#include<ntddk.h>

void PrintLog(char* pszTag, char* pszLevel, char* pszFunc, char* pszFormat, ...);

#define LOG_DEBUG(fmt, ...) (PrintLogW("test", "debug", __FUNCTION__, fmt, ##__VA_ARGS__))

