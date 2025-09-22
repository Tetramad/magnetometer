#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

typedef struct work {
    int priority;
    int (*procedure)(void *);
    void *payload;
} work_t;

enum {
    WORK_PRIORITY_INIT = -1,
    WORK_PRIORITY_DEFAULT,
    WORK_PRIORITY_LOW,
    WORK_PRIORITY_MIN = WORK_PRIORITY_INIT,
    WORK_PRIORITY_MAX = WORK_PRIORITY_LOW,
    WORK_PRIORITY_INVALID = 127,
};

int work_queue_init(void);
int work_queue_enqueue(int priority, int (*procedure)(void *), void *payload);
int work_queue_dequeue(work_t *work);

#endif /* WORK_QUEUE_H */
