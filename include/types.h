#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

enum {
    DIR_NONE,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN
};

typedef enum {
    OK,
    ERROR
} STATUS;

#endif
