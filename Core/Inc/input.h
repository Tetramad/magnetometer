/*
 * input.h
 *
 *      Author: Tetramad
 */

#ifndef __INPUT_H
#define __INPUT_H

typedef struct {
    int left;
    int right;
    int select;
} Input_ContextTypeDef;

int Input_Init(void *payload);
Input_ContextTypeDef Input_NewContext(void);
int Input_UpdateContext(Input_ContextTypeDef *ctx);

#endif /*__INPUT_H*/
