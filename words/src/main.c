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

#define MAX_WORD 16
static float utime = 0;
static char word[MAX_WORD+1]; // + 1 for null terminator
static u8 len = 0;

int main(int argc, char** argv)
{

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

        u8 ch = GetCharPressed();
        while (ch != 0 && len < MAX_WORD-1)
        {
            if (ch != 32)
                word[len++] = ch;
            ch = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && len > 0)
        {
            word[--len] = 0;
        }

        // do e2 draw
        e2_clear(0x0000);

        e2ext_box(2, 2, e2_get_width()-4, 5, 0x30, 0x0c);
        e2_print("these are some words to type", 3, 3, 0x0c);
        e2_print(word, 0, 0, 0x07);

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
