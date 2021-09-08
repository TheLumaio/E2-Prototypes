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

static gui_layout_t* _gui;
static str_t* _text;

static void _hotkey_test()
{
    LOG("hello");
}

static void init();
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
        .title = "EFFECT DEMO"
    };

    bootstrap_e2_t e2 = {
        .width = window.width/10/2,
        .height = window.height/10/2,
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
    _gui = gui_create();
    gui_add_child(_gui, gui_button("TestButton", 1, 4, 0, 0x04, _hotkey_test))->hotkey = 't';

    _text = str_create();
    str_append(_text, 'a');
}

void update()
{

}

void draw()
{
    static const char* cmd = "git rev-parse --short HEAD";
    static char rev[64] = {0};
    
    if (strlen(rev) == 0)
    {
        FILE* out = popen(cmd, "r");
        int ch;
        while ((ch = fgetc(out)) != EOF)
        {
            rev[strlen(rev)] = ch;
        }
        pclose(out);
    }

    e2_rich_print("this is a {#1}rainbow {#0}effect demo", 1, 1);
    e2_rich_print(FormatText("Revision hash is {#1}%s", rev), 1, 2);

    e2_rich_print((const char*)_text->data, 1, 3);

    int c = 0;
    while ((c = GetCharPressed()) != 0)
    {
        str_append(_text, c);
    }

    gui_update(_gui);
    
}


