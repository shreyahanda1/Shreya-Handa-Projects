
// Asgn 2: A simple HTTP server.
// By: Eugene Chou
//     Andrew Quinn
//     Brian Zhao

#include "asgn2_helper_funcs.h"
#include "connection.h"
#include "debug.h"
#include "response.h"
#include "request.h"
#include "queue.h"
#include "hashmap.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/stat.h>

void *producer_thread(void *arg);
void *consumer_thread(void *arg);
void handle_connection(int);
void handle_get(conn_t *);
void handle_put(conn_t *);
void handle_unsupported(conn_t *);

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
sem_t items;
queue_t *my_queue;

volatile sig_atomic_t sr = 0;

//extern pthread_mutex_t mutex;

void signal_handler(int signum) {
    (void) signum;
    sr = 1;
}

int main(int argc, char **argv) {
    signal(SIGINT, signal_handler);
    if (argc < 2) {
        warnx("wrong arguments: %s port_num", argv[0]);
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Use getopt() to parse the command-line arguments */
    int number_of_threads = 4;
    int opt;
    char *endptr = NULL;

    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't':
            // how many threads are there
            number_of_threads = strtoull(optarg, &endptr, 10);
            if (*endptr != '\0' || number_of_threads <= 0) {
                fprintf(stderr, "Invalid Port2\n");
                return EXIT_FAILURE;
            }
            break;
        default: fprintf(stderr, "Invalid Port3\n"); return EXIT_FAILURE;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Invalid Port4\n");
        return EXIT_FAILURE;
    }

    //    char *endptr = NULL;
    size_t port = (size_t) strtoull(argv[optind], &endptr, 10);
    if (endptr && *endptr != '\0') {
        fprintf(stderr, "Invalid Port1\n");
        return EXIT_FAILURE;
    }

    if (port < 1 || port > 65535) {
        fprintf(stderr, "Invalid Port10\n");
        return EXIT_FAILURE;
    }

    signal(SIGPIPE, SIG_IGN);
    Listener_Socket sock;
    if (listener_init(&sock, port) < 0) {
        fprintf(stderr, "Invalid Port11\n");
        return EXIT_FAILURE;
    }

    /* Initialize worker threads, the queue, and other data structures */

    // creating queue to continuously push incoming client requests into it
    my_queue = queue_new(number_of_threads);
    if (my_queue == NULL) {
        fprintf(stderr, "Error\n");
        return EXIT_FAILURE;
    }

    //    sem_init(&items, 0, 0);

    // begins producer thread. accepts connections.
    //    pthread_t producer_thread_init;
    //    pthread_create(&producer_thread_init, NULL, producer_thread, (void *) &sock);

    // begins consumer thread. handles connections.
    pthread_t *consumer_thread_init = malloc(number_of_threads * sizeof(pthread_t));
    for (int i = 0; i < number_of_threads; i++) {
        pthread_create(&consumer_thread_init[i], NULL, consumer_thread, NULL);
    }

    producer_thread(&sock);
    // wait for producer thread to finish before continuing
    /*    pthread_join(producer_thread_init, NULL);

    // join consumer threads together
    for (int i = 0; i < number_of_threads; i++) {
        pthread_join(consumer_thread_init[i], NULL);
    }
*/
    free(consumer_thread_init);
    queue_delete(&my_queue);
    //    sem_destroy(&items);

    /*Hint: You will need to change how handle_connection() is used */
    //    while (1) {
    //        int connfd = listener_accept(&sock);
    //        handle_connection(connfd);
    //        close(connfd);
    //    }

    return EXIT_SUCCESS;
}

// listener thread - producer
void *producer_thread(void *arg) {
    Listener_Socket *sock = (Listener_Socket *) arg;
    while (1) {
        int connfd = listener_accept(sock);
        if (connfd < 0) {
            continue;
        }
        //pthread_mutex_lock(&queue_mutex);
        queue_push(my_queue, (void *) (intptr_t) connfd);
        //pthread_mutex_unlock(&queue_mutex);
        //sem_post(&items); // increment if item is added
    }
    return NULL;
}

// worker thread - consumer
void *consumer_thread(void *arg) {
    (void) arg;
    while (1) {
        //sem_wait(&items); // wait for item to be added
        //        pthread_mutex_lock(&queue_mutex);

        void *connfd_ptr;
        queue_pop(my_queue, &connfd_ptr);
        //        pthread_mutex_unlock(&queue_mutex);

        int connfd = (intptr_t) connfd_ptr;
        handle_connection(connfd);
        close(connfd);
    }
    return NULL;
}

void handle_connection(int connfd) {
    conn_t *conn = conn_new(connfd);

    const Response_t *res = conn_parse(conn);

    if (res != NULL) {
        conn_send_response(conn, res);
    } else {
        //        debug("%s", conn_str(conn));
        const Request_t *req = conn_get_request(conn);
        if (req == &REQUEST_GET) {
            handle_get(conn);
        } else if (req == &REQUEST_PUT) {
            handle_put(conn);
        } else {
            handle_unsupported(conn);
        }
    }

    conn_delete(&conn);
}

void handle_get(conn_t *conn) {
    char *uri = conn_get_uri(conn);
    //    debug("Handling GET request for %s", uri);
    //    fprintf(stderr, "Requesting GET for URI: %s\n", uri);

    // What are the steps in here?

    char *req_id = conn_get_header(conn, "Request-Id");
    if (req_id == NULL) {
        req_id = "0";
    }

    // searching for uri is critical region

    pthread_mutex_lock(&queue_mutex);
    rwlock_t *flock = get_rwlock(uri);
    //    pthread_mutex_lock(&queue_mutex);

    //    pthread_mutex_unlock(&mutex);

    if (flock == NULL) {
        //  pthread_mutex_unlock(&mutex);
        conn_send_response(conn, &RESPONSE_INTERNAL_SERVER_ERROR);
        return;
    }

    reader_lock(flock);

    pthread_mutex_unlock(&queue_mutex);

    // 1. Open the file.
    // If open() returns < 0, then use the result appropriately
    //   a. Cannot access -- use RESPONSE_FORBIDDEN
    //   b. Cannot find the file -- use RESPONSE_NOT_FOUND
    //   c. Other error? -- use RESPONSE_INTERNAL_SERVER_ERROR
    // (Hint: Check errno for these cases)!

    int fd = open(uri, O_RDONLY);
    if (fd < 0) {
        const Response_t *response;
        switch (errno) {
        case EACCES:
            response = &RESPONSE_FORBIDDEN;
            conn_send_response(conn, response);
            fprintf(stderr, "GET,%s,403,%s\n", uri, req_id);
        case ENOENT:
            response = &RESPONSE_NOT_FOUND;
            conn_send_response(conn, response);
            fprintf(stderr, "GET,%s,404,%s\n", uri, req_id);
            reader_unlock(flock);
            return;
        default:
            response = &RESPONSE_INTERNAL_SERVER_ERROR;
            conn_send_response(conn, response);
            fprintf(stderr, "GET,%s,500,%s\n", uri, req_id);
        }
        reader_unlock(flock);
        return;
    }

    //pthread_mutex_unlock(&queue_mutex);

    // 2. Get the size of the file.
    // (Hint: Checkout the function fstat())!

    struct stat size;
    if (fstat(fd, &size) < 0 || S_ISDIR(size.st_mode)) {
        close(fd);
        if (fstat(fd, &size) < 0) {
            conn_send_response(conn, &RESPONSE_INTERNAL_SERVER_ERROR);
            fprintf(stderr, "GET,%s,500,%s\n", uri, req_id);

            // 3. Check if the file is a directory, because directories *will*
            // open, but are not valid.
            // (Hint: Checkout the macro "S_IFDIR", which you can use after you call fstat()!)

        } else if (S_ISDIR(size.st_mode)) {
            conn_send_response(conn, &RESPONSE_FORBIDDEN);
            fprintf(stderr, "GET,%s,403,%s\n", uri, req_id);
        }
        reader_unlock(flock);
        return;
    }

    // 4. Send the file
    // (Hint: Checkout the conn_send_file() function!)

    conn_send_file(conn, fd, size.st_size);
    fprintf(stderr, "GET,%s,200,%s\n", uri, req_id);

    // 5. Close the file

    close(fd);
    reader_unlock(flock);
}

void handle_put(conn_t *conn) {
    char *uri = conn_get_uri(conn);
    const Response_t *res = NULL;
    //   debug("Handling PUT request for %s", uri);

    // What are the steps in here?

    char *req_id = conn_get_header(conn, "Request-Id");
    if (req_id == NULL) {
        req_id = "0";
    }

    pthread_mutex_lock(&queue_mutex);
    rwlock_t *flock = get_rwlock(uri);

    //   pthread_mutex_unlock(&mutex);

    if (flock == NULL) {
        conn_send_response(conn, &RESPONSE_INTERNAL_SERVER_ERROR);
        return;
    }

    writer_lock(flock);

    //    pthread_mutex_unlock(&queue_mutex);

    //    pthread_mutex_unlock(&mutex);

    // 1. Check if file already exists before opening it.
    // (Hint: check the access() function)!

    //    pthread_mutex_unlock(&queue_mutex);
    if (access(uri, F_OK | R_OK) == -1) {
        res = &RESPONSE_CREATED;
    }

    int fd;
    struct stat exists;

    if (stat(uri, &exists) == 0) { // checks if file exists
        if (S_ISDIR(exists.st_mode)) { // checks if directory
            conn_send_response(conn, &RESPONSE_FORBIDDEN);
            fprintf(stderr, "PUT,%s,403,%s\n", uri, req_id);
            writer_unlock(flock);
            return;
        }

        // 2. Open the file.
        // If open() returns < 0, then use the result appropriately
        //   a. Cannot access -- use RESPONSE_FORBIDDEN
        //   b. File is a directory -- use RESPONSE_FORBIDDEN
        //   c. Cannot find the file -- use RESPONSE_FORBIDDEN
        //   d. Other error? -- use RESPONSE_INTERNAL_SERVER_ERROR
        // (Hint: Check errno for these cases)!

        fd = open(uri, O_WRONLY | O_TRUNC); // open existing file that you can write to

    } else { // file does not exist
        if (errno == ENOENT) {
            fd = open(uri, O_WRONLY | O_CREAT, 0644); // if does not exist, creates the file
        } else {
            conn_send_response(conn, &RESPONSE_INTERNAL_SERVER_ERROR);
            fprintf(stderr, "PUT,%s,500,%s\n", uri, req_id);
            writer_unlock(flock);
            return;
        }
    }

    pthread_mutex_unlock(&queue_mutex);
    if (fd < 0) {
        const Response_t *response;
        switch (errno) {
        case EACCES:
            response = &RESPONSE_FORBIDDEN;
            conn_send_response(conn, response);
            fprintf(stderr, "PUT,%s,403,%s\n", uri, req_id);
        case EISDIR:
            response = &RESPONSE_FORBIDDEN;
            conn_send_response(conn, response);
            fprintf(stderr, "PUT,%s,403,%s\n", uri, req_id);
        default:
            response = &RESPONSE_INTERNAL_SERVER_ERROR;
            conn_send_response(conn, response);
            fprintf(stderr, "PUT,%s,500,%s\n", uri, req_id);
        }
        writer_unlock(flock);
        return;
    }

    //3. Receive the file
    // (Hint: Checkout the conn_recv_file() function)!

    const Response_t *response = conn_recv_file(conn, fd);

    if (response != NULL) {
        conn_send_response(conn, response);
        writer_unlock(flock);
        return;
    }

    // 4. Send the response
    // (Hint: Checkout the conn_send_response() function)!
    if (res != &RESPONSE_CREATED) {
        conn_send_response(conn, &RESPONSE_OK);
        fprintf(stderr, "PUT,%s,200,%s\n", uri, req_id);
        writer_unlock(flock);
        return;
    } else {
        conn_send_response(conn, &RESPONSE_CREATED);
        fprintf(stderr, "PUT,%s,201,%s\n", uri, req_id);
        writer_unlock(flock);
        return;
    }

    // 5. Close the file

    close(fd);
    writer_unlock(flock);
}

void handle_unsupported(conn_t *conn) {
    debug("Handling unsupported request");

    // Send responses
    conn_send_response(conn, &RESPONSE_NOT_IMPLEMENTED);
}
