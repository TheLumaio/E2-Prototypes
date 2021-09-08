#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <raylib.h>

#include "e2.h"
#include "g1.h"

#include "bootstrap.h"

static void init();
static void update();
static void draw();

int main(int argc, char** argv)
{
    bootstrap_window_t window = {
        .log_level = LOG_NONE,
        .window_flags = FLAG_WINDOW_RESIZABLE,
        .fps_max = 60,
        .exit_key = KEY_F12,
        .width = 640,
        .height = 480,
        .title = "TEMPLATE"
    };

    bootstrap_e2_t e2 = {
        .width = window.width/10,
        .height = window.height/10,
        .update = update,
        .draw = draw
    };

    bootstrap_init(&window, &e2);
    init();
    bootstrap_run(); 
    bootstrap_close();

    return 0;
}

void init()
{
    
}

void update()
{

}

void draw()
{

}


