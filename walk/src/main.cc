#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

#include <raylib.h>
#include <raymath.h>
#include <glad/glad.h>

extern "C"
{
#include "taffer.h"

#include "e2.h"
#include "g2.h"
}

typedef void(*func_ptr)();

static void drop_menu(u16 x, u16 y, const char* title, ...)
{
    va_list args;
    va_start(args, title);

    int count = 0;
    char* a = va_arg(args, char*);
    while (a != NULL) { count++; a = va_arg(args, char*); }
    e2ext_box(x-1, y, 15, count, 0x70, 0x00);

    va_start(args, title);

    e2_print(title, x, y, 0x70);

    char* option_text = va_arg(args, char*);
    int num = 0;
    g2_button("", 0xffff, 0xffff, 0xff, ALIGN_DEFAULT);
    while (option_text != NULL)
    {
        func_ptr callback = va_arg(args, func_ptr);
        if (g2_button(option_text, x, y+1*(num+1), 0xc0, (align_e)0))
        {
            ((func_ptr)callback)();
        }

        num++;
        option_text = va_arg(args, char*);
    }

    va_end(args);
}

void hello()
{
    printf("button called\n");
}

static e2vec2_t _menu_pos = {0, 0};

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

    while (!WindowShouldClose())
    {
        e2_clear(0x0000);

        drop_menu(10, 10, "options", "Hello", hello, "world", hello, NULL);

        BeginDrawing();
        ClearBackground(BLACK);

        e2_render();

        EndDrawing();
    }

    e2_close();

    CloseWindow();

    return 0;

}
