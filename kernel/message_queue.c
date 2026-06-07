/*
 * message_queue.c
 *
 * MELK OS - Static blocking message queue service
 *
 * Properties of this first implementation:
 * - FIFO queue with fixed-size messages.
 * - Storage is provided by the application; no dynamic allocation.
 * - send() blocks when the queue is full.
 * - receive() blocks when the queue is empty.
 * - Uses the already validated mutex and counting semaphore services.
 * - Task context only. ISR-safe queue APIs can be added later.
 */

#include "message_queue.h"

static void message_queue_copy_bytes(uint8_t *destination,
                                     const uint8_t *source,
                                     uint32_t size)
{
    uint32_t index;

    for (index = 0U; index < size; index++)
    {
        destination[index] = source[index];
    }
}

static uint8_t *message_queue_slot_address(os_message_queue_t *queue,
                                           uint32_t slot_index)
{
    return &queue->storage[slot_index * queue->message_size];
}

int32_t os_message_queue_init(os_message_queue_t *queue,
                              void *storage,
                              uint32_t message_size,
                              uint32_t capacity)
{
    int32_t mutex_status;
    int32_t items_status;
    int32_t spaces_status;

    if ((queue == 0) || (storage == 0))
    {
        return OS_MESSAGE_QUEUE_ERROR_NULL_POINTER;
    }

    if ((message_size == 0U) || (capacity == 0U))
    {
        return OS_MESSAGE_QUEUE_ERROR_INVALID_CONFIG;
    }

    queue->storage = (uint8_t *)storage;
    queue->message_size = message_size;
    queue->capacity = capacity;
    queue->head = 0U;
    queue->tail = 0U;
    queue->is_initialized = OS_MESSAGE_QUEUE_NOT_INITIALIZED;

    mutex_status = os_mutex_init(&queue->access_mutex);

    if (mutex_status != OS_MUTEX_OK)
    {
        return OS_MESSAGE_QUEUE_ERROR_SYNC_OPERATION;
    }

    items_status = os_semaphore_init(&queue->available_items, 0U, capacity);

    if (items_status != OS_SEMAPHORE_OK)
    {
        return OS_MESSAGE_QUEUE_ERROR_SYNC_OPERATION;
    }

    spaces_status = os_semaphore_init(&queue->available_spaces,
                                      capacity,
                                      capacity);

    if (spaces_status != OS_SEMAPHORE_OK)
    {
        return OS_MESSAGE_QUEUE_ERROR_SYNC_OPERATION;
    }

    queue->is_initialized = OS_MESSAGE_QUEUE_INITIALIZED;

    return OS_MESSAGE_QUEUE_OK;
}

int32_t os_message_queue_send(os_message_queue_t *queue,
                              const void *message)
{
    uint8_t *destination;
    uint32_t next_tail;
    int32_t wait_status;
    int32_t mutex_status;
    int32_t unlock_status;
    int32_t post_status;

    if ((queue == 0) || (message == 0))
    {
        return OS_MESSAGE_QUEUE_ERROR_NULL_POINTER;
    }

    if (queue->is_initialized != OS_MESSAGE_QUEUE_INITIALIZED)
    {
        return OS_MESSAGE_QUEUE_ERROR_NOT_INITIALIZED;
    }

    wait_status = os_semaphore_wait(&queue->available_spaces);

    if (wait_status != OS_SEMAPHORE_OK)
    {
        return OS_MESSAGE_QUEUE_ERROR_SYNC_OPERATION;
    }

    mutex_status = os_mutex_lock(&queue->access_mutex);

    if (mutex_status != OS_MUTEX_OK)
    {
        (void)os_semaphore_post(&queue->available_spaces);
        return OS_MESSAGE_QUEUE_ERROR_SYNC_OPERATION;
    }

    destination = message_queue_slot_address(queue, queue->tail);
    message_queue_copy_bytes(destination,
                             (const uint8_t *)message,
                             queue->message_size);

    next_tail = queue->tail + 1U;

    if (next_tail >= queue->capacity)
    {
        next_tail = 0U;
    }

    queue->tail = next_tail;

    unlock_status = os_mutex_unlock(&queue->access_mutex);

    if (unlock_status != OS_MUTEX_OK)
    {
        return OS_MESSAGE_QUEUE_ERROR_SYNC_OPERATION;
    }

    post_status = os_semaphore_post(&queue->available_items);

    if (post_status != OS_SEMAPHORE_OK)
    {
        return OS_MESSAGE_QUEUE_ERROR_SYNC_OPERATION;
    }

    return OS_MESSAGE_QUEUE_OK;
}

int32_t os_message_queue_receive(os_message_queue_t *queue,
                                 void *message)
{
    uint8_t *source;
    uint32_t next_head;
    int32_t wait_status;
    int32_t mutex_status;
    int32_t unlock_status;
    int32_t post_status;

    if ((queue == 0) || (message == 0))
    {
        return OS_MESSAGE_QUEUE_ERROR_NULL_POINTER;
    }

    if (queue->is_initialized != OS_MESSAGE_QUEUE_INITIALIZED)
    {
        return OS_MESSAGE_QUEUE_ERROR_NOT_INITIALIZED;
    }

    wait_status = os_semaphore_wait(&queue->available_items);

    if (wait_status != OS_SEMAPHORE_OK)
    {
        return OS_MESSAGE_QUEUE_ERROR_SYNC_OPERATION;
    }

    mutex_status = os_mutex_lock(&queue->access_mutex);

    if (mutex_status != OS_MUTEX_OK)
    {
        (void)os_semaphore_post(&queue->available_items);
        return OS_MESSAGE_QUEUE_ERROR_SYNC_OPERATION;
    }

    source = message_queue_slot_address(queue, queue->head);
    message_queue_copy_bytes((uint8_t *)message,
                             source,
                             queue->message_size);

    next_head = queue->head + 1U;

    if (next_head >= queue->capacity)
    {
        next_head = 0U;
    }

    queue->head = next_head;

    unlock_status = os_mutex_unlock(&queue->access_mutex);

    if (unlock_status != OS_MUTEX_OK)
    {
        return OS_MESSAGE_QUEUE_ERROR_SYNC_OPERATION;
    }

    post_status = os_semaphore_post(&queue->available_spaces);

    if (post_status != OS_SEMAPHORE_OK)
    {
        return OS_MESSAGE_QUEUE_ERROR_SYNC_OPERATION;
    }

    return OS_MESSAGE_QUEUE_OK;
}
