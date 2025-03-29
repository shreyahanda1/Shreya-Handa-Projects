## Goal

Experience managing concurrency through synchronization, dealing with multiple threads. Nearly all modern systems are faced with managing concurrency to increase their hardware utilization.

While my server processes multiple clients simultaneously, it also ensures that its responses conform to a coherent and atomic linearization of the client requests. In effect, this means that an outside observer could not differentiate the behavior of my server from a server that uses only a single thread.

My server also creates an audit log that identifies the linearization of my server. Log entries in my audit log are also Atomic (i.e., there are not any partial, overwritten, or otherwise corrupted lines in my log). The log is durable: my server ensures that the log entries for each request processed by my server are resident in the log.

## Ordering Requirements

My server produces responses that are coherent and atomic with respect to the ordering specified in my audit log. That is, if an entry for a request R1, is earlier than an entry for a request, R2, in the log, then the server’s response to R2 must be as though the processing for R1 occurred in its entirety before any of the processing for R2. We call this ordering a linearization because it creates a single linear ordering of all client requests. We say that this ordering is a total order because it provides an ordering for all elements (i.e., for any two unique requests, R1 or R2, my audit log will identify that either R1 happens-before R2 or that R2 happens-before R1).

My server’s linearization conforms to the order of requests that it received in the following way: if two requests, R1 and R2, arrive such that the start time of R2 is after the end time of R1, then my audit log indicates the line for R2 after the log line for R1. However, if R1 and R2 overlap (i.e., the start time of R1 is before the start time of R2 and the start time of R2 is before the end time of R1), then my server’s audit log could produce audit log entries in either order (i.e., R1 could be before R2 or R2 could be before R1).

## Efficiency

My server is as efficient as possible while still providing the ordering requirement. In other words, my server only blocks request processing if either (1) there are already thread active clients or (2) blocking request processing is necessary to ensure the coherency/atomicity of my server’s linearization of requests. This means that my server concurrently processes requests when it is safe to do so. It cannot simply spawn threads on demand, but only perform work in a single thread.

## Multi-threading

My server uses a thread-pool design, a popular concurrent programming construct for implementing servers. A thread pool has two types of threads, worker threads, which process requests, and a dispatcher thread, which listens for connections and dispatches them to the workers. It makes sense to dispatch requests by having the dispatcher push them into a threadsafe queue (Assignment 3, see Resources below) and having worker threads pop elements from that queue.

**Worker Threads**  
My server creates exactly threads worker threads (Note: this means that the server has exactly threads + 1 threads). A worker thread is idle (i.e., sleeping by waiting on a lock, conditional variable, or semaphore) if there are no requests to be processed. Each worker thread performs HTTP processing for a request (Assignment 1, see Resources below). I carefully implement synchronization to ensure that my server properly maintains a state that is shared across worker threads and to ensure coherency and atomicity.

**Dispatcher Thread**  
My server uses a single dispatcher thread. A typical design uses the main thread (i.e., the function call main), as the dispatcher. The dispatcher waits for connections from a client. Once a connection is initiated, the dispatcher alerts one of the worker threads and listens for a new client. If there are no idle worker threads, then the dispatcher waits until a worker thread finishes its current task. My server implements correct synchronization to ensure that the dispatcher thread and worker threads correctly “hand-off” client requests without dropping any client requests, corrupting any data, or crashing the server.

## Examples

**Example 1**  
This example focuses on the audit log format. Suppose that the server receives the following requests, one after the other, and that a.txt exists initially, but b.txt does not:

```
GET /a.txt HTTP/1.1\r\nRequest-Id: 1\r\n\r\n
GET /b.txt HTTP/1.1\r\nRequest-Id: 2\r\n\r\n
PUT /b.txt HTTP/1.1\r\nRequest-Id: 3\r\nContent-Length: 3\r\n\r\nbye
GET /b.txt HTTP/1.1\r\n\r\n
```

The server should produce the audit log:
```
GET,/a.txt,200,1
GET,/b.txt,404,2
PUT,/b.txt,201,3
GET,/b.txt,200,0
```

**Example 2**  
This example focuses on times when my server operates totally concurrently—i.e., when it processes multiple requests at the same time.

Suppose that I start my server with two threads in a directory containing the files a.txt, b.txt, and c.txt. My server creates two worker threads and one dispatcher thread. The worker threads wait for a message from the dispatcher, while the dispatcher thread waits for a connection on its listen socket. Then, suppose that the following three requests arrive at my server concurrently (i.e., at the same time):

```
GET /a.txt HTTP/1.1\r\nRequest-Id: 1\r\n\r\n
GET /b.txt HTTP/1.1\r\nRequest-Id: 2\r\n\r\n
GET /c.txt HTTP/1.1\r\nRequest-Id: 3\r\n\r\n
```

The dispatcher thread wakes up one of the worker threads to handle one of the requests and wakes up another worker thread to handle a second of the requests. The dispatcher tracks that it has seen, but not processed, the third request. Request processing synchronizes as little as possible—in this case, since the requests do not access the same URI, the processing does not need to synchronize at all. As soon as either worker finishes processing their request, that worker begins processing the third request. The other thread waits to be alerted by the dispatcher that there are more requests.

Because the requests are operated concurrently, my server can produce any of the following six audit logs:

```
GET,/a.txt,200,1
GET,/b.txt,200,2
GET,/c.txt,200,3

GET,/a.txt,200,1
GET,/c.txt,200,3
GET,/b.txt,200,2

GET,/b.txt,200,2
GET,/a.txt,200,1
GET,/c.txt,200,3

GET,/b.txt,200,2
GET,/c.txt,200,3
GET,/a.txt,200,1

GET,/c.txt,200,3
GET,/a.txt,200,1
GET,/b.txt,200,2

GET,/c.txt,200,3
GET,/b.txt,200,2
GET,/a.txt,200,1
```

After all of this processing, the server should be in the steady state of having (1) the workers waiting for alerts from the dispatcher and (2) the dispatcher thread waiting for a connection (i.e., calling listener accept() on the listen socket).

**Example 3**  
This example identifies scenarios when my server’s request processing synchronizes.

Suppose that I start my server with two threads in a directory that does not contain the file, a.txt. My server creates two worker threads and one dispatcher thread. The worker threads wait for a message from the dispatcher, while the dispatcher thread waits for a connection on its listen socket.

Suppose that the following two requests arrive at my server concurrently:

```
GET /a.txt HTTP/1.1\r\nRequest-Id: 1\r\n\r\n
PUT /a.txt HTTP/1.1\r\nRequest-Id: 2\r\nContent-Length: 3\r\n\r\nbye
```

My server produces an audit log and responses to requests that are consistent with each other. Namely, there are only two possible combinations of audit log and response:

**Option 1**  
Audit Log:
```
GET,/goodbye.txt,404,1
PUT,/goodbye.txt,201,2
```

Request Response:
```
1 HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n
2 HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated\n
```

**Option 2**  
Audit Log:
```
PUT,/goodbye.txt,201,2
GET,/goodbye.txt,200,1
```

Request Response:
```
2 HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated\n
1 HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nOK\n
```

After all of this processing, the server should be back in the steady state of having (1) the workers waiting for requests to arrive and (2) the dispatcher thread waiting for a connection (i.e., calling accept() on the listen socket).
