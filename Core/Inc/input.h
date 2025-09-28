/*
 * input.h
 *
 *      Author: Tetramad
 */

#ifndef __INPUT_H
#define __INPUT_H

struct InputContext {
    int left;
    int right;
    int select;
};

int Input_Init(void *payload);
struct InputContext Input_NewContext(void);
int Input_UpdateContext(struct InputContext *ctx);

#endif /*__INPUT_H*/
