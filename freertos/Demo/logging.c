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

void ReleaseLog(char* old_name, char* new_name, int now)
{
    if(strcmp(old_name,new_name))
    {
        vLoggingPrintf(  "[ %d ] %s release\n", now, old_name );

        vLoggingPrintf(  "[ %d ] %s start\n", now, new_name );
    }
}
