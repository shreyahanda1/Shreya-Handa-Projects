## Goal

Building a system that requires synchronization between multiple threads.

I built both a bounded buffer and a reader-writer lock for this assignment. Both fulfill defined APIs and meet certain behavior requirements.

## Queue Functionality

I implemented 4 functions to fulfill the bounded buffer API. The API defines a queue, having FIFO properties, which stores and returns arbitrary pointers to objects. This means that I can store anything in the queue. Their API is as follows:

```c
typedef struct queue queue_t;
queue_t *queue_new(int size);
void queue_delete(queue_t **q);
bool queue_push(queue_t *q, void *elem);
bool queue_pop(queue_t *q, void **elem);
```

## Reader-Writer Lock Functionality

A reader/writer lock, rwlock, allows multiple readers to hold the lock at the same time, but only a single writer. I implement 6 functions to fulfill the reader-writer lock API. The goal of a reader-writer lock is to ensure that the throughput of reading an item is unchanged if there are no writers present. The shortcoming of a traditional reader-writer lock is that they can starve readers or writers when there is contention. I will develop a scheme that allows for a PRIORITY argument that decides how the rwlock behaves when there is contention, favoring either: readers, writers, or n-way.

```c
typedef struct rwlock rwlock_t;
typedef enum {READERS, WRITERS, N_WAY} PRIORITY;
rwlock_t* = rwlock_new(PRIORITY p, int n);
void rwlock_delete(rwlock_t **l);
void reader_lock(rwlock_t *rw);
void reader_unlock(rwlock_t *rw);
void writer_lock(rwlock_t *rw);
void writer_unlock(rwlock_t *rw);
```