#include "logging.h"

void vLoggingPrintf( const char * pcFormat,
                     ... )
{
    char* string;

    va_list arg;

    va_start( arg, pcFormat );
    vsprintf(string, pcFormat, arg);
    va_end( arg );
    UART_printf(string);
}

void ReleaseLog(char* issuer_name, char* target_name, int now)
{
    vLoggingPrintf("[ %d ] %s released %s\n", now, issuer_name, target_name);
}