/*
 * work_queue.h
 *
 *      Author: Tetramad
 */

#ifndef __WORK_QUEUE_H
#define __WORK_QUEUE_H

typedef struct {
    int priority;
    int (*procedure)(void *);
    void *payload;
} WorkTypeDef;

enum {
    WORK_PRIORITY_INIT = -1,
    WORK_PRIORITY_DEFAULT,
    WORK_PRIORITY_LOW,
    WORK_PRIORITY_MIN = WORK_PRIORITY_INIT,
    WORK_PRIORITY_MAX = WORK_PRIORITY_LOW,
    WORK_PRIORITY_INVALID = 127,
};

int WorkQueue_Init(void);
int WorkQueue_Enqueue(int priority, int (*procedure)(void *), void *payload);
int WorkQueue_Dequeue(WorkTypeDef *work);

#endif /* __WORK_QUEUE_H */
