#include <stddef.h>

#include <stm32f4xx_hal.h>

#include <log.h>
#include <macros.h>
#include <work_queue.h>

#define MAX_WORK_NUM 12

LOG_LEVEL_SET(LOG_LEVEL_INF);

static void BubbleSort(void);

WorkTypeDef works[MAX_WORK_NUM] = {0};

int WorkQueue_Init(void) {
    for (size_t i = 0; i < ARRAY_SIZE(works); ++i) {
        works[i].priority = WORK_PRIORITY_INVALID;
    }

    return 0;
}

int WorkQueue_Enqueue(int priority, int (*procedure)(void *), void *payload) {
    int result = -1;

    __disable_irq();
    for (size_t i = 0; i < ARRAY_SIZE(works); ++i) {
        if (works[i].priority == WORK_PRIORITY_INVALID) {
            works[i] = (WorkTypeDef){.priority = priority,
                                     .procedure = procedure,
                                     .payload = payload};
            result = 0;
            break;
        }
    }
    BubbleSort();
    __enable_irq();

    return result;
}

int WorkQueue_Dequeue(WorkTypeDef *work) {
    if (works[0].priority == WORK_PRIORITY_INVALID) {
        return -1;
    }

    __disable_irq();
    *work = works[0];
    works[0] = (WorkTypeDef){.priority = WORK_PRIORITY_INVALID,
                             .procedure = NULL,
                             .payload = NULL};
    BubbleSort();
    __enable_irq();

    return 0;
}

static void BubbleSort(void) {
    for (size_t _ = 0; _ < ARRAY_SIZE(works); ++_) {
        int sorted = 1;

        for (size_t i = 1; i < ARRAY_SIZE(works); ++i) {
            if (works[i - 1].priority > works[i].priority) {
                WorkTypeDef temp = works[i - 1];
                works[i - 1] = works[i];
                works[i] = temp;
                sorted = 0;
            }
        }

        if (sorted) {
            break;
        }
    }
}
