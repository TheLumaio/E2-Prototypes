#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <math.h>
#include "e2.h"

#define RUN_COUNT 64

typedef struct _run_t {
    int pos;
    int len;
    float speed;
    float life;
} _run_t;

static _run_t* _runs;
static float _time;
static int _speed;

void matrix_init(int speed);
void matrix_run();

#endif
