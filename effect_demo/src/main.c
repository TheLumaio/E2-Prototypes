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

static void update();
static void draw();

int main(int argc, char** argv)
{
    bootstrap_window_t window = {
        .log_level = LOG_ALL,
        .window_flags = FLAG_WINDOW_RESIZABLE,
        .fps_max = 60,
        .exit_key = KEY_F12,
        .width = 640,
        .height = 480,
        .title = "TEMPLATE"
    };

    bootstrap_e2_t e2 = {
        .width = window.width/10/2,
        .height = window.height/10/2,
        .update = update,
        .draw = draw
    };

    bootstrap_init(&window, &e2);
    bootstrap_run(); 
    bootstrap_close();

    return 0;
}

void update()
{

}

void draw()
{
    e2_rich_print("this is a {#1}rainbow {#0}effect demo", 1, 1);

    static bool n = false;
    if (IsKeyPressed(KEY_SPACE)) n = !n;
    if (!n) return;

    static const char* trippy = "\xdctrippy\xdc";
    static u16 index = 0;

    e2_putc(trippy[index%strlen(trippy)], index%e2_get_width(), index/e2_get_width(), 0x07);
    e2_effect_set(index%e2_get_width(), index/e2_get_width(), EFFECT_RAINBOW);
    index = (index+1)%(e2_get_width()*e2_get_height());
}


