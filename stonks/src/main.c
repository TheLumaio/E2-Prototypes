#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>


#include <raylib.h>
#include <raymath.h>
#include <glad/glad.h>

#undef ShowWindow

#include "taffer.h"
#include "e2.h"

int main(int argc, char** argv)
{

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(640, 480, "E2");
    SetTargetFPS(0);
    SetExitKey(KEY_F12);

    Image i = (Image){(void*)taffer, 160, 160, 1, 7};
    Texture texture = LoadTextureFromImage(i);

    e2_init(texture, GetScreenWidth()/10, GetScreenHeight()/10);

    while (!WindowShouldClose())
    {
        e2_clear(0x0000);

        e2_print("!!STONKS TIME!!", 0, 0, 0x80);

        BeginDrawing();
        ClearBackground(BLACK);

        e2_render();

        EndDrawing();
    }

    e2_close();

    CloseWindow();

    return 0;

}
