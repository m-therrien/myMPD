/* myMPD
   (c) 2018-2019 Juergen Mang <mail@jcgames.de>
   This project's homepage is: https://github.com/jcorporation/mympd
   
   This linked list implementation is based on: https://github.com/joshkunz/ashuffle
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <asm/errno.h>
#include "tiny_queue.h"

tiny_queue_t *tiny_queue_create(void) {
    struct tiny_queue_t* queue = (struct tiny_queue_t*)malloc(sizeof(struct tiny_queue_t));
    queue->head = NULL;
    queue->tail = NULL;
    queue->length = 0;

    queue->mutex  = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    queue->wakeup = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    return queue;
}

void tiny_queue_free(tiny_queue_t *queue) {
    struct tiny_msg_t *current_head = queue->head, *tmp = NULL;
    while (current_head != NULL) {
        free(current_head->data);
        tmp = current_head;
        current_head = current_head->next;
        free(tmp);
    }
    free(queue);
}


int tiny_queue_push(tiny_queue_t *queue, void *data) {
    int rc = pthread_mutex_lock(&queue->mutex);
    if (rc != 0) {
        printf("Error in pthread_mutex_lock: %d\n", rc);
        return 0;
    }
    struct tiny_msg_t* new_node = (struct tiny_msg_t*)malloc(sizeof(struct tiny_msg_t));
    new_node->data = data;
    new_node->next = NULL;
    queue->length++;
    if (queue->head == NULL && queue->tail == NULL){
        queue->head = queue->tail = new_node;
    }
    else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
    rc = pthread_mutex_unlock(&queue->mutex);
    if (rc != 0) {
        printf("Error in pthread_mutex_unlock: %d\n", rc);
        return 0;
    }
    rc = pthread_cond_signal(&queue->wakeup);
    if (rc != 0) {
        printf("Error in pthread_cond_signal: %d\n", rc);
        return 0;
    }
    return 1;
}

int tiny_queue_length(tiny_queue_t *queue, int timeout) {
    timeout = timeout * 1000;  
    int rc = pthread_mutex_lock(&queue->mutex);
    if (rc != 0) {
        printf("Error in pthread_mutex_lock: %d\n", rc);
        return 0;
    }
    if (timeout > 0 && queue->length == 0) {
        struct timespec max_wait = {0, 0};
        clock_gettime(CLOCK_REALTIME, &max_wait);
        //timeout in ms
        if (max_wait.tv_nsec <= (999999999 - timeout)) {
            max_wait.tv_nsec += timeout;
        } else {
            max_wait.tv_sec += 1;
            max_wait.tv_nsec = timeout - (999999999 - max_wait.tv_nsec);
        }
        rc = pthread_cond_timedwait(&queue->wakeup, &queue->mutex, &max_wait);
        if (rc != 0 && rc != ETIMEDOUT) {
            printf("Error in pthread_cond_timedwait: %d\n", rc);
        }
    }
    unsigned len = queue->length;
    rc = pthread_mutex_unlock(&queue->mutex);
    if (rc != 0) {
        printf("Error in pthread_mutex_unlock: %d\n", rc);
    }
    return len;
}

void *tiny_queue_shift(tiny_queue_t *queue, int timeout) {
    timeout = timeout * 1000;
    int rc = pthread_mutex_lock(&queue->mutex);
    if (rc != 0) {
        printf("Error in pthread_mutex_lock: %d\n", rc);
        return 0;
    }
    if (queue->length == 0) {
        if (timeout > 0) {
            struct timespec max_wait = {0, 0};
            clock_gettime(CLOCK_REALTIME, &max_wait);
            //timeout in ms
            if (max_wait.tv_nsec <= (999999999 - timeout)) {
                max_wait.tv_nsec += timeout;
            } else {
                max_wait.tv_sec += 1;
                max_wait.tv_nsec = timeout - (999999999 - max_wait.tv_nsec);
            }
            rc = pthread_cond_timedwait(&queue->wakeup, &queue->mutex, &max_wait);
            if (rc != 0) {
                if (rc != ETIMEDOUT) {
                    printf("Error in pthread_cond_timedwait: %d\n", rc);
                    printf("nsec: %ld\n", max_wait.tv_nsec);
                }
                rc = pthread_mutex_unlock(&queue->mutex);
                if (rc != 0) {
                    printf("Error in pthread_mutex_unlock: %d\n", rc);
                }
                return NULL;
            }
        }
        else {
            rc = pthread_cond_wait(&queue->wakeup, &queue->mutex);
            if (rc != 0) {
                printf("Error in pthread_cond_wait: %d\n", rc);
                rc = pthread_mutex_unlock(&queue->mutex);
                if (rc != 0) {
                    printf("Error in pthread_mutex_unlock: %d\n", rc);
                }
                return NULL;
            }
        }
    }
    //queue has entry
    if (queue->head != NULL) {
        struct tiny_msg_t* current_head = queue->head;
        void *data = current_head->data;
        if (queue->head == queue->tail) {
            queue->head = queue->tail = NULL;
        }
        else {
            queue->head = queue->head->next;
        }
        free(current_head);
        queue->length--;
        rc = pthread_mutex_unlock(&queue->mutex);
        if (rc != 0) {
            printf("Error in pthread_mutex_unlock: %d\n", rc);
        }
        return data;
    }
    else {
        rc = pthread_mutex_unlock(&queue->mutex);
        if (rc != 0) {
            printf("Error in pthread_mutex_unlock: %d\n", rc);
        }
        return NULL;
    }
}
