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

#include "state.h"
#include "statelist.h"

static Shader post_process;
static u8 pp_time;
static u8 pp_resolution;

static void init();
static void update();
static void draw();

static void override()
{

    // override frame rendering
    Texture frame;

    update();
    BeginDrawing();
    draw();
    e2_render();
    frame = e2_get_framebuffer();
    BeginShaderMode(post_process);
    float time = GetTime();
    Vector2 res = (Vector2){GetScreenWidth(), GetScreenHeight()};
    SetShaderValue(post_process, pp_time, &time, UNIFORM_FLOAT);
    SetShaderValue(post_process, pp_resolution, &res, UNIFORM_VEC2);
    DrawTexturePro(_e2_framebuffer.texture,
                   (Rectangle){0, 0, _e2_framebuffer.texture.width,
                               _e2_framebuffer.texture.height},
                   (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
                   (Vector2){0, 0}, 0, WHITE);
    EndShaderMode();
    EndDrawing();
}

int main(int argc, char** argv)
{
    bootstrap_window_t window = {
        .log_level = LOG_ALL,
        .window_flags = FLAG_WINDOW_RESIZABLE,
        .fps_max = 60,
        .exit_key = KEY_F12,
        .width = 800,
        .height = 480,
        .title = "SUBSTANCE"
    };

    bootstrap_e2_t e2 = {
        .width = window.width/10,
        .height = window.height/10,
        .update = update,
        .draw = draw,
//        .override = override
    };

    bootstrap_init(&window, &e2);
    init();
    bootstrap_run();
    bootstrap_close();

    return 0;
}

void init()
{
    post_process = LoadShader(NULL, "post_process.glsl");
    pp_time = GetShaderLocation(post_process, "time");
    pp_resolution = GetShaderLocation(post_process, "resolution");

    CHANGE_STATE(login);

    //ToggleFullscreen();

}

void update()
{
    state_update();
}

void draw()
{
    state_draw();
}
