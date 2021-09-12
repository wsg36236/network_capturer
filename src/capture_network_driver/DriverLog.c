#include "DriverLog.h"
#include <ntstrsafe.h>

void PrintLogW(char* pszTag, char* pszLevel, char* pszFunc, char* pszFormat, ...)
{
    wchar_t buffer[1024] = { 0 };

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    va_list argptr;
    va_start(argptr, pszFormat);
    NTSTATUS status = RtlStringCchPrintfA(buffer, sizeof(buffer) / sizeof(buffer[0]), pszFormat, argptr);
    va_end(argptr);

    DbgPrint("[%s][%u][%u][%s][%s] %s\n", pszTag, PsGetCurrentProcessId(), PsGetCurrentThreadId(), pszLevel, pszFunc, buffer);
}
