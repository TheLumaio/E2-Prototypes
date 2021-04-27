#ifndef CUTSCENE_H
#define CUTSCENE_H

#include <stdio.h>
#include "list.h"
#include "e2.h"
#include "clock.h"

typedef enum _effect_e {
    _EFFECT_NONE,
    _EFFECT_RAINBOW,
    _EFFECT_PAUSE
} _effect_e;

typedef struct _segment_t {
    char* text;
    _effect_e effect;
    float speed;
} _segment_t;

void scene_parse_script(const char* script);
void scene_step();


#endif

