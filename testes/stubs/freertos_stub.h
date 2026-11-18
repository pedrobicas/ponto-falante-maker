// FreeRTOS mínimo (só o que o firmware usa). A tarefa do MVP não roda no PC.
#pragma once

#include <cstdint>

typedef void* QueueHandle_t;
typedef void* SemaphoreHandle_t;
typedef uint32_t TickType_t;
typedef int BaseType_t;
typedef void (*TaskFunction_t)(void*);
typedef void* TaskHandle_t;

#define pdTRUE 1
#define pdFALSE 0
#define portMAX_DELAY 0xffffffffUL
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))

SemaphoreHandle_t xSemaphoreCreateMutex();
BaseType_t xSemaphoreTake(SemaphoreHandle_t, TickType_t);
BaseType_t xSemaphoreGive(SemaphoreHandle_t);
QueueHandle_t xQueueCreate(uint32_t tamanho, uint32_t itemBytes);
BaseType_t xQueueSend(QueueHandle_t, const void* item, TickType_t);
BaseType_t xQueueReceive(QueueHandle_t, void* item, TickType_t);
BaseType_t xTaskCreatePinnedToCore(TaskFunction_t, const char*, uint32_t, void*, unsigned, TaskHandle_t*, int);
