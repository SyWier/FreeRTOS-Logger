//
// Created by Daniel on 2026-10-31.
//

#include "logger.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stdio.h"
#include "stdarg.h"

// Log config
#define LOG_MESSAGE_SIZE 128
#define LOG_QUEUE_SIZE 10
#define LOG_STACK_SIZE 256
#define LOG_TIMEOUT_MS 100
#define LOG_FORMAT_SIZE 28 // Color(7) + Timestamp(15) + ColorReset(4) + NewLine(1) + Zero(1) = 28 bytes
#define LOG_ENABLE_TIMESTAMP 1

// Message struct
struct message_t {
	TickType_t timestamp;
	char level;
	char text[LOG_MESSAGE_SIZE];
};

// Private variables
static QueueHandle_t log_queue = NULL;
static UART_HandleTypeDef *log_huart = NULL;
static char log_out_buf[LOG_MESSAGE_SIZE + LOG_FORMAT_SIZE]; // Message + Format

static const char *level_colors[] = {
	"\x1b[1;31m", // Red     - ERROR
	"\x1b[1;33m", // Yellow  - WARNING
	"\x1b[1;32m", // Cyan    - INFO
	"\x1b[0;37m", // Gray    - DEBUG
};

static const char *reset_color = "\x1b[0m";

// Function declerations
static void log_task(void *argument);

// Function implementations

/**
 * @brief Initializes the logger queue, task, and UART handle.
 * @param huart UART handle used for log output.
 * @return true on success, false if already initialized or resource creation fails.
 */
bool log_init(UART_HandleTypeDef *huart)
{
	if (log_huart != NULL)
		return false;

	// Save handle
	log_huart = huart;

	// Create queue
	log_queue = xQueueCreate(LOG_QUEUE_SIZE, sizeof(struct message_t));
	if (log_queue == NULL)
		return false;

	// Create task
	BaseType_t result = xTaskCreate(log_task, "log_task", LOG_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
	if (result != pdPASS) {
		// Cleanup
		vQueueDelete(log_queue);
		log_queue = NULL;
		return false;
	}

	return true;
}


/**
 * @brief Formats and queues a log message without waiting for queue space.
 * @param level Message severity.
 * @param tag Source identifier prepended to the message.
 * @param fmt Printf-style format string.
 * @param ... Format arguments.
 * @note Task context only. Oversized message text is truncated.
 */
void log_message(char level, const char *tag, const char *fmt, ...)
{
	// Verify initialization and input pointers
	if (log_huart == NULL || log_queue == NULL || fmt == NULL) {
		return;
	}

	// Capture the event timestamp and its severity
	struct message_t message ;
	message.level = level;
#if LOG_ENABLE_TIMESTAMP == 1
	message.timestamp = pdMS_TO_TICKS(xTaskGetTickCount());
#endif

	// Prepend the source tag
	int offset = 0;
	if (tag[0] != '\0') {
		offset = snprintf(message.text, LOG_MESSAGE_SIZE, "[%s] ", tag);
	}

	// Reject formatting errors or a truncated tag prefix
	if (offset < 0 || offset >= LOG_MESSAGE_SIZE) {
		return;
	}

	// Format the message into the remaining buffer space
	va_list args;
	va_start(args, fmt);
	const int result = vsnprintf(message.text + offset, LOG_MESSAGE_SIZE - offset, fmt, args);
	va_end(args);

	if (result < 0) {
		return;
	}

	// Copy the message into the queue without blocking
	xQueueSend(log_queue, &message, 0);
}


void log_task(void *argument)
{
	(void) argument;

	struct message_t message;

	while (true) {
		if (xQueueReceive(log_queue, &message, portMAX_DELAY) == pdPASS) {
			// Validate level value
			if (message.level < 0 || message.level > LOG_DBG) {
				continue;
			}

#if LOG_ENABLE_TIMESTAMP == 1
			int length = snprintf(log_out_buf, sizeof(log_out_buf), "%s[%lu ms]%s%s\n", level_colors[message.level],
			                  message.timestamp, message.text, reset_color);
#else
			int length = snprintf(log_out_buf, sizeof(log_out_buf), "%s%s%s\n", level_colors[message.level], message.text,
			                  reset_color);
#endif

			// snprinf error
			if (length < 0) {
				continue;
			}

			// Truncate message
			const int size = length < sizeof(log_out_buf) ? length : sizeof(log_out_buf) - 1;

			// Send message via UART
			HAL_UART_Transmit(log_huart, (uint8_t *) (log_out_buf), size, LOG_TIMEOUT_MS);
		}
	}
}
