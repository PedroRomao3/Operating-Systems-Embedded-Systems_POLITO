#include "logging.h"
#include "uart.h"

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

void ReleaseLog(char* old, char* new, int old_release_time)
{
    if(strcmp(old,new))
    {
        vLoggingPrintf(  "[ %d ] %s release\n", old_release_time, old );

        vLoggingPrintf(  "[ %d ] %s start\n", xTaskGetTickCount(), new );
    }
}
