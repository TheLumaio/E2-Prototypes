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
#include "taffer.h"
#include "e2.h"
#include "e2_ext.h"

#include "player.h"

static float utime = 0;
static u8 _plot_size = 20;

int main(int argc, char** argv)
{

    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(640, 480, "E2");
    SetTargetFPS(0);
    SetExitKey(KEY_F12);

    Image i = (Image){(void*)taffer, 160, 160, 1, 7};
    Texture texture = LoadTextureFromImage(i);

    e2_init(texture, GetScreenWidth()/10, GetScreenHeight()/10);

    char input[17];

    while (!WindowShouldClose())
    {
        // do update
        utime += GetFrameTime();

        if (utime >= 0.1)
        {

            utime -= 0.1;
        }

        // do e2 draw
        e2_clear(0x0000);

        e2ext_box(e2_get_width()/2-_plot_size/2, e2_get_height()/2-_plot_size/2, _plot_size, _plot_size, 0x70, 0x01);

        if (g2_button_box("harvest", 28, 10, 0xd1, 0x70, 0x01, 0))
        {
            player_pay();
        }

        // e2ext_box(1, 1, e2_get_width()/2-_plot_size/2-2, e2_get_height()-2, 0x70, 0x03);
        // e2ext_box(e2_get_width()/2+_plot_size/2+1, 1, e2_get_width()/2-_plot_size/2-2, e2_get_height()-2, 0x70, 0x03);
        e2_rich_print(FormatText("{0xc0}${0x70}%d", (int)player_get_dosh()), 3, 3);

        // do raylib draw
        BeginDrawing();
        ClearBackground(BLACK);

        e2_render();

        EndDrawing();
    }

    e2_close();

    CloseWindow();

    return 0;

}
