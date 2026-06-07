/*
 * message_queue.h
 *
 * MELK OS - Static blocking message queue service
 */

#ifndef KERNEL_MESSAGE_QUEUE_H_
#define KERNEL_MESSAGE_QUEUE_H_

#include <stdint.h>
#include "mutex.h"
#include "semaphore.h"

#define OS_MESSAGE_QUEUE_OK                         0
#define OS_MESSAGE_QUEUE_ERROR_NULL_POINTER        -1
#define OS_MESSAGE_QUEUE_ERROR_INVALID_CONFIG      -2
#define OS_MESSAGE_QUEUE_ERROR_NOT_INITIALIZED     -3
#define OS_MESSAGE_QUEUE_ERROR_SYNC_OPERATION      -4

#define OS_MESSAGE_QUEUE_NOT_INITIALIZED            0U
#define OS_MESSAGE_QUEUE_INITIALIZED                1U

typedef struct
{
    uint8_t *storage;
    uint32_t message_size;
    uint32_t capacity;

    volatile uint32_t head;
    volatile uint32_t tail;

    os_mutex_t access_mutex;
    os_semaphore_t available_items;
    os_semaphore_t available_spaces;

    volatile uint32_t is_initialized;
} os_message_queue_t;

int32_t os_message_queue_init(os_message_queue_t *queue,
                              void *storage,
                              uint32_t message_size,
                              uint32_t capacity);

int32_t os_message_queue_send(os_message_queue_t *queue,
                              const void *message);

int32_t os_message_queue_receive(os_message_queue_t *queue,
                                 void *message);

#endif /* KERNEL_MESSAGE_QUEUE_H_ */
