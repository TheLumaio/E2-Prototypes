#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <raylib.h>
#include <raymath.h>
#include <glad/glad.h>

#include "taffer.h"

#include "e2.h"
#include "matrix.h"

static Color _colors[16] = {
    (Color){0, 0, 0, 255},        // 0x0
    (Color){0, 40, 0, 255},       // 0x1
    (Color){0, 56, 0, 255},       // 0x2
    (Color){0, 88, 0, 255},       // 0x3
    (Color){0, 184, 0, 255},      // 0x4
    (Color){0, 216, 0, 255},      // 0x5
    (Color){0, 232, 0, 255},      // 0x6
    (Color){0, 248, 0, 255},      // 0x7
    (Color){171, 70, 66, 255},    // 0x8
    (Color){220, 150, 86, 255},   // 0x9
    (Color){247, 202, 136, 255},  // 0xa
    (Color){161, 181, 108, 255},  // 0xb
    (Color){121, 135, 79, 255},   // 0xc
    (Color){134, 193, 185, 255},  // 0xd
    (Color){124, 175, 194, 255},  // 0xe
    (Color){161, 105, 70, 255}    // 0xf
};

int main(int argc, char** argv)
{

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(640, 480, "E2");
    SetTargetFPS(0);
    SetExitKey(KEY_F12);

    Image i = (Image){(void*)taffer, 160, 160, 1, 7};
    Texture texture = LoadTextureFromImage(i);

    e2_init(texture, GetScreenWidth()/10, GetScreenHeight()/10);
    e2_set_palette(_colors);

    matrix_init(20);

    while (!WindowShouldClose())
    {
        e2_clear(0x0000);

        BeginDrawing();
        ClearBackground(BLACK);

        matrix_run();

        Vector2 mpos = e2_get_mouse();
        e2_print("Debug info", 0, 0, 0x07);
        e2_rich_print(FormatText("{0x07}FPS: {0x0d}%d", GetFPS()), 0, 1);
        e2_rich_print(FormatText("MousePos: {0x0c}%d %d", (int)mpos.x, (int)mpos.y), 0, 2);
        e2_rich_print(FormatText("Resolution: %dx%d", GetScreenWidth(), GetScreenHeight()), 0, 3);

        e2_render();

        EndDrawing();
    }

    e2_close();

    CloseWindow();

    return 0;

}
