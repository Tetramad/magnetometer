#include <stddef.h>

#include <stm32f4xx.h>

#include <log.h>
#include <macros.h>
#include <work_queue.h>

static void bubble_sort(void);

work_t work_queue[12] = {0};

int work_queue_init(void) {
    for (size_t i = 0; i < ARRAY_SIZE(work_queue); ++i) {
        work_queue[i].priority = WORK_PRIORITY_INVALID;
    }

    return 0;
}

int work_queue_enqueue(int priority, int (*procedure)(void *), void *payload) {
    int result = -1;

    __disable_irq();
    for (size_t i = 0; i < ARRAY_SIZE(work_queue); ++i) {
        if (work_queue[i].priority == WORK_PRIORITY_INVALID) {
            work_queue[i] = (work_t){.priority = priority,
                                     .procedure = procedure,
                                     .payload = payload};
            result = 0;
            break;
        }
    }
    bubble_sort();
    __enable_irq();

    return result;
}

int work_queue_dequeue(work_t *work) {
    if (work_queue[0].priority == WORK_PRIORITY_INVALID) {
        return -1;
    }

    __disable_irq();
    *work = work_queue[0];
    work_queue[0] = (work_t){.priority = WORK_PRIORITY_INVALID,
                             .procedure = NULL,
                             .payload = NULL};
    bubble_sort();
    __enable_irq();

    return 0;
}

static void bubble_sort(void) {
    for (size_t _ = 0; _ < ARRAY_SIZE(work_queue); ++_) {
        int sorted = 1;

        for (size_t i = 1; i < ARRAY_SIZE(work_queue); ++i) {
            if (work_queue[i - 1].priority > work_queue[i].priority) {
                work_t temp = work_queue[i - 1];
                work_queue[i - 1] = work_queue[i];
                work_queue[i] = temp;
                sorted = 0;
            }
        }

        if (sorted) {
            break;
        }
    }
}
