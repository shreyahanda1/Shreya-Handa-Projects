#include <pthread.h>
#include "rwlock.h"
#include "uthash.h" // provides hash table purposes. used from github
#include <stdio.h>
#include "httpserver.h"

typedef struct {
    char *uri;
    rwlock_t *rwlock;
    UT_hash_handle hh;
} rwlock_struct;

rwlock_struct *lock = NULL; // pointer to beginning of hashtable
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // mutex for thread-safe implementation

// checks if there is another lock entry for the uri. if there isn't, then creates a new lock and adds to hashtable
// each file has its own lock
rwlock_t *get_rwlock(const char *uri) {
    // locks the mutex
    //    fprintf(stderr, "[get_rwlock]: Attempting to acquire global mutex for URI: %s\n", uri);
    //    pthread_mutex_lock(&mutex);
    //    fprintf(stderr, "[get_rwlock]: Global mutex acquired for URI: %s\n", uri);

    rwlock_struct *lock_check = NULL;

    // finds lock for uri entry
    HASH_FIND_STR(lock, uri, lock_check);

    /*    if (lock_check != NULL) {
        fprintf(stderr, "[get_rwlock]: Found existing rwlock for URI: %s\n", uri);
    }*/

    // if there is no lock entry found, creates one
    if (lock_check == NULL) {
        //        pthread_mutex_unlock(&mutex);
        //        fprintf(stderr, "[get_rwlock]: No rwlock found for URI: %s, creating new...\n", uri);
        lock_check = malloc(sizeof(rwlock_struct));
        if (lock_check == NULL) {
            //            fprintf(stderr, "[get_rwlock]: Failed to allocate memory for new rwlock_struct\n");
            //            free(lock_check);
            //            pthread_mutex_unlock(&mutex);
            return NULL;
        }

        lock_check->uri = strdup(uri);
        if (lock_check->uri == NULL) {
            //            fprintf(stderr, "[get_rwlock]: Failed to duplicate URI string\n");
            free(lock_check);
            //            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        // readers priority
        lock_check->rwlock = rwlock_new(READERS, 0);
        if (lock_check->rwlock == NULL) {
            //            fprintf(stderr, "[get_rwlock]: Failed to create new rwlock\n");

            free(lock_check->uri);
            free(lock_check);
            //            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        // pthread_mutex_lock(&mutex);
        HASH_ADD_KEYPTR(hh, lock, lock_check->uri, strlen(lock_check->uri), lock_check);
        //        fprintf(stderr, "[get_rwlock]: New rwlock created and added to hash for URI: %s\n", uri);
    }
    // otherwise return the lock if it already exists
    rwlock_t *rw = lock_check->rwlock;
    //    pthread_mutex_unlock(&mutex);
    //    fprintf(stderr, "[get_rwlock]: Global mutex released for URI: %s\n", uri);

    return rw;
}