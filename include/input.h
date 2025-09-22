#ifndef INPUT_H
#define INPUT_H

struct input_context {
    int left;
    int right;
    int select;
};

int input_init(void *payload);
struct input_context input_new_context(void);
int input_update_context(struct input_context *ctx);

#endif /*INPUT_H*/
