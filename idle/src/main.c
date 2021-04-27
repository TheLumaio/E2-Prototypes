#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <raylib.h>
#include <raymath.h>
#include <glad/glad.h>

#include "g2.h"
#include "e2_ext.h"

#include "bootstrap.h"

#define new(...) malloc(__VA_ARGS__)

#define CUR_TREE 'T'
#define CUR_LIGHT 'L'
#define CUR_NONE 0

#define FARM_SIZE 10

static u64 _coins = 0;
static u16 _auto = 0;

static u8 _cursor = CUR_NONE;

static float _time = 0;

static u8 _farm[FARM_SIZE*FARM_SIZE];

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
        .title = "IDLE"
    };

    bootstrap_e2_t e2 = {
        .width = window.width/10/3,
        .height = window.height/10/3,
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
    _time += GetFrameTime();
    if (_time >= 1)
    {
        _coins += _auto;
        _time -= 1;
    }
}

void draw()
{
    e2_clear(0x0000);

    e2_rich_print(FormatText("{0x09}%20d{0x0f}$", _coins), 0, 0);
    e2_rich_print(FormatText("{0x0a}%20d{0x0c}t", _auto), 0, 1);
    
    if (g2_button("C", 0, 1, 0x0c, ALIGN_DEFAULT))
    {
        _coins++;
    }
    if (g2_button("D", 1, 1, 0x0c, ALIGN_DEFAULT))
    {
        if (_coins >= 100)
        {
            _coins -= 100;
            _auto++;
        }
    }
    if (g2_button("T", 2, 1, 0x0c, ALIGN_DEFAULT))
    {
        _cursor = CUR_TREE;
    }
    else if (g2_button("L", 3, 1, 0x0c, ALIGN_DEFAULT))
    {
        _cursor = CUR_LIGHT;
    }
    else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        _cursor = CUR_NONE;
    }
    

    Vector2 mpos = e2_get_mouse();

    e2ext_box(5, 3, 12, 12, 0x07, 0x30);
    for (int x = 0; x < FARM_SIZE; x++)
    {
        for (int y = 0; y < FARM_SIZE; y++)
        {
            if (_farm[y*FARM_SIZE+x]!=0)
                e2_putc(_farm[y*FARM_SIZE+x], 6+x, 4+y, 0x3d);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (mpos.x == 6+x && mpos.y == 4+y)
                {
                    switch (_cursor)
                    {
                    case CUR_TREE:
                        if (_coins >= 1000)
                        {
                            _farm[y * FARM_SIZE + x] = 'T';
                            _coins -= 1000;
                        }
                        break;
                    case CUR_LIGHT:
                        _farm[y*FARM_SIZE+x] = 'L';
                        break;
                    case CUR_NONE:
                    default:
                        break;
                    }
                    _coins -= 1;
                }
            }
        }
    }
    
    if (mpos.x > 5 && mpos.x < 16 && mpos.y > 3 && mpos.y < 14)
    {
        e2_putc('O', mpos.x, mpos.y, 0x37);
    }
}


