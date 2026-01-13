#include "logging.h"

#include "FreeRTOSConfig.h"
#include "semphr.h"

void vLoggingPrintf( const char * pcFormat, ... )
{
    static SemaphoreHandle_t logMutex;
    if (!logMutex)
        logMutex = xSemaphoreCreateMutex();
    
    xSemaphoreTake(logMutex, portMAX_DELAY);
    
    static char buffer[configLOG_BUFFER_SIZE];

    va_list arg;

    va_start( arg, pcFormat );
    vsnprintf(buffer, sizeof(buffer), pcFormat, arg);
    va_end( arg );
    UART_printf(buffer);

    xSemaphoreGive(logMutex);
}

void ReleaseLog(char* issuer_name, char* target_name, int now)
{
    vLoggingPrintf("[ %d ] %s released %s\n", now, issuer_name, target_name);
}