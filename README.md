# FreeRTOS Logger

A lightweight C logging service for STM32 projects using FreeRTOS and STM32 HAL. The calling task formats each message and queues it. A dedicated logger task sends queued messages over UART.

## Usage

1. Add `logger.c` to your firmware target and add the directory containing `logger.h` to its include paths.
2. Initialize the UART, then initialize the logger with its UART handle before calling the logging macros.
3. Include `logger.h` in each source file that emits logs. Define `LOG_LOCAL_LEVEL` before the first include only if that file needs to override `LOG_DEFAULT_LEVEL`. The tag is optional too.

Initialize the logger once during application startup. This example uses the CubeMX-generated USART2 handle.

```c
#include "logger.h"
#include "usart.h"

void app_init(void)
{
    /* Call after MX_USART2_UART_Init(). */
    logger_init(&huart2);
}
```

In a source file that uses the logger:

```c
#define LOG_TAG "DeviceManager"
#define LOG_LOCAL_LEVEL LOG_DBG  /* Optional */
#include "logger.h"

void report_slot_error(int slot)
{
    log_error("Slot %d failed", slot);
    log_debug("Check the slot connection");
}
```

Use the logging macros from task context. The available levels, from least to most verbose, are `LOG_ERR`, `LOG_WRN`, `LOG_INF`, and `LOG_DBG`.

## Configuration

| Define | Purpose |
| --- | --- |
| `LOG_TAG` | Optional per-file message prefix. |
| `LOG_LOCAL_LEVEL` | Optional per-file compile-time log level. |
| `LOG_DEFAULT_LEVEL` | Compile-time fallback for files without a local override. |
| `LOG_MESSAGE_SIZE` | Message buffer size in bytes. |
| `LOG_QUEUE_SIZE` | Maximum number of queued messages. |
| `LOG_STACK_SIZE` | Stack depth of the logger task. |
| `LOG_TIMEOUT_MS` | UART transmit timeout in milliseconds. |
| `LOG_ENABLE_TIMESTAMP` | Enables or disables timestamps. |

The last five settings are defined in `logger.c`.
