#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <raylib.h>

#include "e2.h"
#include "g1.h"
#include "str.h"

#include "bootstrap.h"

#include "cutscene.h"

static u8 _cursor_x = 0;
static u8 _cursor_y = 0;
static str_t* _str = NULL;

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
        .title = "TEXT"
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
    _str = str_create();
    scene_parse_script("hello world\n");
}

void update()
{
}

void draw()
{

    e2_putc(0xdb, _cursor_x, _cursor_y, 0x07);

}


