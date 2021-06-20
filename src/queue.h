/**
* \file queue.h
*/

#ifndef H_QUEUE
#define H_QUEUE

#include <stdbool.h>
#include "types.h"

/**
* \typedef queue
* \brief FILO queue with data of type 'direction'.
*/
struct myqueue {
    direction* data;
    int size;
    int start;
    int end;
    bool is_full;
};

myqueue new_queue(int size);
bool myqueue_empty(myqueue* q);
bool myqueue_full(myqueue* q);
void myenqueue(myqueue* q, direction elt);
direction mydequeue(myqueue* q);
void mydisplay_queue_int(myqueue q);
void myfree_queue(myqueue* q);

#endif

