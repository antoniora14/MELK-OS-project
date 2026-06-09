# Static Message Queue Service

A message queue is a kernel communication primitive used to transfer fixed-size messages between tasks.

MELK OS implements a static blocking FIFO message queue.

---

## Purpose

Use a message queue when one task needs to send data to another task.

Examples:

- Measurements
- Commands
- Events
- CAN frames
- Log entries

---

## Basic API

```c
int32_t os_message_queue_init(os_message_queue_t *queue,
                              void *storage,
                              uint32_t message_size,
                              uint32_t capacity);

int32_t os_message_queue_send(os_message_queue_t *queue,
                              const void *message);

int32_t os_message_queue_receive(os_message_queue_t *queue,
                                 void *message);
```

---

## Static Storage Model

The application provides the message storage buffer.

Example:

```c
#define QUEUE_CAPACITY 4U

typedef struct
{
    uint32_t producer_id;
    uint32_t sequence;
    uint32_t tick;
} demo_message_t;

static demo_message_t g_queue_storage[QUEUE_CAPACITY];
static os_message_queue_t g_queue;

os_message_queue_init(&g_queue,
                      g_queue_storage,
                      sizeof(demo_message_t),
                      QUEUE_CAPACITY);
```

No dynamic memory allocation is used.

---

## Internal Design

The message queue uses:

```text
1 circular buffer
1 mutex
2 semaphores
```

### Circular Buffer

```text
storage[]
head
tail
capacity
message_size
```

### Mutex

Protects internal queue data:

```text
head
tail
storage[]
```

### Semaphores

```text
available_items  → number of messages ready to read
available_spaces → number of free slots available for writing
```

---

## Send Flow

```text
os_message_queue_send()
    ↓
wait(available_spaces)
    ↓
lock(access_mutex)
    ↓
copy message into storage[tail]
    ↓
advance tail
    ↓
unlock(access_mutex)
    ↓
post(available_items)
```

If the queue is full, `available_spaces` is zero and the producer blocks.

---

## Receive Flow

```text
os_message_queue_receive()
    ↓
wait(available_items)
    ↓
lock(access_mutex)
    ↓
copy message from storage[head]
    ↓
advance head
    ↓
unlock(access_mutex)
    ↓
post(available_spaces)
```

If the queue is empty, `available_items` is zero and the consumer blocks.

---

## Producer/Consumer Model

Example:

```text
Producer Task 1 → sends measurements
Producer Task 2 → sends events
Consumer Task 3 → receives and processes messages
```

The queue preserves FIFO order for messages inserted into it.

---

## Why Both Mutex and Semaphores?

The semaphores decide whether the operation can proceed:

```text
available_items  → can receive?
available_spaces → can send?
```

The mutex protects the actual internal structure while modifying:

```text
head
tail
storage
```

Without the mutex, two producers could write to the same `tail` position or corrupt the circular buffer.

---

## Summary

Use message queue when the problem is:

```text
One task needs to send data to another task.
```

A queue is more than a signal. It carries actual data.
