#include "logging.h"

#include "FreeRTOSConfig.h"
#include "semphr.h"
#include "message_buffer.h"

#if !LOG_USE_BUFFERING

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

#else


    static StaticStreamBuffer_t xLogStreamStruct;
    static uint8_t ucLogStorage[ configLOG_BUFFER_SIZE + 1 ];
    static StreamBufferHandle_t xLogStream;

    void vLoggingQueue( const char* pcFormat, ...)
    {
        // NOTE(Reda): Dont think a semaphore here is required since we are not doing in place printing and we are already buffering
        // TODO(Reda): Add a limit on the messaging queue for how many messages could be queued at once
     
        if (!xLogStream)
        {
            xLogStream = xStreamBufferCreateStatic(
                sizeof(ucLogStorage),
                1,                      // trigger level (bytes)
                ucLogStorage,
                &xLogStreamStruct
            );
            configASSERT(xLogStream);
        }

        if (xLogStream == NULL)
            return;

        // static char buffer[configLOG_BUFFER_SIZE];
        char line[128]; // TODO: Move to configLOG_LINE_MAX

        va_list arg;

        va_start( arg, pcFormat );
        int n = vsnprintf(line, sizeof(line), pcFormat, arg);
        va_end( arg );

        if (n <= 0) return;

        // Non-blocking send: if no space, drop
        int sent = (int)xStreamBufferSend(xLogStream, line, n, 0);
        // if (sent < len) {
        //     ulLogDroppedBytes += (uint32_t)(len - sent);
    }

    void vLoggingTask(void *pvParameters)
    {
            (void)pvParameters;

        uint8_t rx[128];

        for (;;)
        {
            size_t n = xStreamBufferReceive(xLogStream, rx, sizeof(rx), portMAX_DELAY);
            if (n > 0) {
                UART_printf((char*)&rx[0]);
            }
        }
    }

#endif