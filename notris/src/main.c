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

#include "clock.h"

#define NOTRIS_WIDTH 10
#define NOTRIS_HEIGHT 20
#define NOTRIS_X 7
#define NOTRIS_Y 2

static u8 _board[NOTRIS_WIDTH*NOTRIS_HEIGHT];
static u32 _score = 0;
static u8 _level = 1;
static u8 _combo = 0;

static const u16 _notriminos[7][4] = {
    // stolen from https://stackoverflow.com/a/38596291
    {0x44C0, 0x8E00, 0xC880, 0xE200}, // 'J'
    {0x88C0, 0xE800, 0xC440, 0x2E00}, // 'L'
    {0x8C40, 0x6C00, 0x8C40, 0x6C00}, // 'S'
    {0x4640, 0x0E40, 0x4C40, 0x4E00}, // 'T'
    {0x4C80, 0xC600, 0x4C80, 0xC600}, // 'Z'
    {0x4444, 0x0F00, 0x4444, 0x0F00}, // 'I'
    {0xCC00, 0xCC00, 0xCC00, 0xCC00}  // 'O'
};
static const u8 _notri_color_map[7] = {
    0x09,
    0x0a,
    0x0b,
    0x0c,
    0x0d,
    0x0e,
    0x0f
};

static const i8 _jlstz_seeds[8*2] = {
    0, 1,
    1, 0,
    1, 2,
    2, 1,
    2, 3,
    3, 2,
    3, 0,
    0, 3
};
static const i8 _jlstz_offsets[8 * 5 * 2] = {
    0, 0, -1, 0, -1, 1, 0, -2, -1, 2,
    0, 0, 1, 0, 1, -1, 0, 2, 1, 2,
    0, 0, 1, 0, 1, -1, 0, 2, 1, 2,
    0, 0, -1, 0, -1, 1, 0, -2, -1, -2,
    0, 0, 1, 0, 1, 1, 0, -2, 1, -2,
    0, 0, -1, 0, -1, -1, 0, 2, -1, 2,
    0, 0, 1, 0, 1, 1, 0, -2, 1, -2
};
static const i8 _i_offsets[8 * 5 * 2] = {
    0, 0, -2, 0, 1, 0, -2, -1, 1, 2,
    0, 0, 2, 0, -1, 0, 2, 1, -1, -2,
    0, 0, -1, 0, 2, 0, -1, 2, 2, -1,
    0, 0, 1, 0, -2, 0, 1, -2, -2, 1,
    0, 0, 2, 0, -1, 0, 2, 1, -1, -2,
    0, 0, -2, 0, 1, 0, -2, -1, 1, 2,
    0, 0, 1, 0, -2, 0, 1, -2, -2, 1,
    0, 0, -1, 0, 2, 0, -1, 2, 2, -1
};

static u8 _mino = 0;
static u8 _rot = 0;
static u8 _next = 1;
static i8 _hold = -1;

static i8 _minox = 4;
static i8 _minoy = 0;
static eclock_t* _main_clock = NULL;
static bool _paused = false;
static bool _gameover = false;
static bool _swapped = false;

static Sound _snd_clear;
static Sound _snd_gameover;
static Sound _snd_hold;
static Sound _snd_place;
static Sound _snd_move;

static void update();
static void draw();

static void copy_to_board(u8 mino, u8 rot)
{
    u16 notrimino = _notriminos[mino][rot];
    for (u8 i = 0; i < 16; i++) {
        u8 x = i % 4;
        u8 y = i / 4;
        u16 t = notrimino & (0x8000 >> i);
        if (t) {
          u8 boardx = _minox + x;
          u8 boardy = _minoy + y;
          _board[boardy * NOTRIS_WIDTH + boardx] = _mino + 1;
        }
    }

    PlaySound(_snd_place);
}

static bool check_offset(i8 offx, i8 offy)
{
    u16 notrimino = _notriminos[_mino][_rot];
    for (u8 i = 0; i < 16; i++) {
        u8 x = i % 4;
        u8 y = i / 4;
        u16 t = notrimino & (0x8000 >> i);
        if (t) {
            i8 boardx = _minox + x + offx;
            i8 boardy = _minoy + y + offy;
            if (_board[boardy * NOTRIS_WIDTH + boardx] != 0 || boardx < 0 || boardx >= NOTRIS_WIDTH || boardy == NOTRIS_HEIGHT)
            {
                return true;
            }
        }
    }
    return false;
}

static void wall_kick(u8);
static void get_next_mino() {
    _minoy = 0;
    _minox = 4;
    _rot = 0;
    _mino = _next;
    _swapped = false;
        
    _next = rand() % 7;

    _main_clock->timer = 0;

    wall_kick(0);
}

static void tick_notris(clock_t* clock)
{
    // bottom out
    if (check_offset(0, 1))
    {
        copy_to_board(_mino, _rot);
        get_next_mino();
    }

    // top out
    if (check_offset(0, 0)) {
        PlaySound(_snd_gameover);
        _gameover = true;
        _main_clock->paused = true;
    }

    _minoy++;
}

static void wall_kick(u8 nrot)
{
    if (_mino == 6) return;

    i8 oldx = _minox;
    i8 oldy = _minoy;
    i8 oldr = _rot;

    for (u8 seed = 0; seed < 8; seed++)
    {
        u8 seed_l = _jlstz_seeds[seed*2+0];
        u8 seed_r = _jlstz_seeds[seed*2+1];

        u8* arr = _jlstz_offsets;
        if (_mino == 5)
            arr = _i_offsets;
        
        if (_rot == seed_l && nrot == seed_r) {
            for (u8 i = 0; i < 5; i++)
            {
                
                _minox = oldx + arr[seed*10+i*2+0];
                _minoy = oldy + arr[seed*10+i*2+1];
                _rot = nrot;
                if (!check_offset(0, 0)) {
                    _main_clock->timer = 0;
                    return;
                }
            }
        }
    }
    
    _minox = oldx;
    _minoy = oldy;
    _rot = oldr;
}



int main(int argc, char** argv)
{
    bootstrap_window_t window = {
        .log_level = LOG_ALL,
        .window_flags = FLAG_WINDOW_RESIZABLE,
        .fps_max = 60,
        .exit_key = KEY_F12,
        .width = 24*10*3,
        .height = 24*10*3,
        .title = "NOTRIS" // Alexey Pajitnov is a capitalist traitor
    };

    bootstrap_e2_t e2 = {
        .width = 24,
        .height = 24,
        .char_w = 10,
        .char_h = 10,
        .font_file = NULL,//"font.png",
        .update = update,
        .draw = draw
    };

    bootstrap_init(&window, &e2);

    clock_init_pool();
    _main_clock = clock_create(300, tick_notris);
    _mino = rand() % 7;
    _next = rand() % 7;

    SetMasterVolume(0.5);

    _snd_clear = LoadSound("clear.wav");
    _snd_gameover = LoadSound("gameover.wav");
    _snd_hold = LoadSound("hold.wav");
    _snd_place = LoadSound("place.wav");
    _snd_move = LoadSound("move.wav");

    bootstrap_run(); 
    bootstrap_close();

    return 0;
}

void update()
{

    clock_update_pool(GetFrameTime());

    if (IsKeyPressed(KEY_LEFT) && !_paused && !_gameover)
    {
        if (!check_offset(-1, 0)) {
            PlaySound(_snd_move);
            _minox--;
        }
    }
    if (IsKeyPressed(KEY_RIGHT) && !_paused && !_gameover)
    {
        if (!check_offset(1, 0)) {
            PlaySound(_snd_move);
            _minox++;
        }
    }
    if (IsKeyPressed(KEY_X) && !_paused && !_gameover)
    {
        wall_kick((_rot + 1) % 4);
    }
    if (IsKeyPressed(KEY_Z) && !_paused && !_gameover)
    {
        u8 nrot = _rot;
        if (nrot == 0)
            nrot = 3;
        else
            nrot--;
        wall_kick(nrot);
    }

    if (IsKeyPressed(KEY_P) && !_gameover)
    {
        _paused = !_paused;
        _main_clock->paused = !_main_clock->paused;
    }
    if (IsKeyDown(KEY_DOWN) && !_paused && !_gameover)
    {
        _main_clock->timer = 999;
    } else if (IsKeyReleased(KEY_DOWN))
        _main_clock->timer = 0;

    if (IsKeyPressed(KEY_UP) && !_paused && !_gameover)
    {
        // hard drop
        while (!check_offset(0, 1)) {
            _minoy++;
        }
        copy_to_board(_mino, _rot);
        get_next_mino();
    }

    if (IsKeyPressed(KEY_R) && IsKeyDown(KEY_LEFT_CONTROL))
    {
        if (!_gameover)
        {
            PlaySound(_snd_gameover);
        }
        
        get_next_mino();
        memset(_board, 0, sizeof(_board));
        _score = 0;
        _level = 1;
        _main_clock->timer = 0;
        _main_clock->paused = false;
        _gameover = false;
        _hold = -1;
    }

    if (IsKeyPressed(KEY_SPACE) && !_swapped)
    {
        if (_hold < 0)
        {
            _hold = _mino;
            get_next_mino();
        }
        else
        {
            u8 temp = _mino;
            _mino = _hold;
            _hold = temp;

            _minox = 4;
            _minoy = 0;
            _rot = 0;
            _main_clock->timer = 0;
        }

        PlaySound(_snd_hold);

        _swapped = true;

    }

}

void draw()
{
    e2_clear(0x8800);

    e2_rich_print(FormatText("{0x0c}SCORE {0x07}%d", _score), 0, 0);
    e2ext_box(NOTRIS_X - 1, NOTRIS_Y - 1, NOTRIS_WIDTH + 2, NOTRIS_HEIGHT + 2, 0x07, 0x00);

    u8 lines_complete = 0;
    for (u8 i = 0; i < NOTRIS_WIDTH * NOTRIS_HEIGHT; i++)
    {
        u8 x = i % NOTRIS_WIDTH;
        u8 y = i / NOTRIS_WIDTH;
        u8 color = 0x01;
        if (_board[i] != 0)
            color = _notri_color_map[_board[i]-1];
        e2_putc(0xdb, NOTRIS_X + x, NOTRIS_Y + y, color);

        // clear lines
        if (x != 0) continue;
        bool complete = true;
        for (u8 j = 0; j < NOTRIS_WIDTH; j++)
        {
            if (_board[y * NOTRIS_WIDTH + j] == 0)
            {
                complete = false;
                break;
            }
        }
        if (!complete) continue;
        lines_complete++;
        memmove((&_board[0])+NOTRIS_WIDTH, &_board[0], i);
        i = 0;
    }

    switch (lines_complete)
    {
    case 0:
        break;
    case 1:
        _score += 40 * _level * (_combo + 1);
        break;
    case 2:
        _score += 100 * _level * (_combo + 1);
        break;
    case 3:
        _score += 300 * _level * (_combo + 1);
        break;
    case 4:
        _score += 1200 * _level * (_combo + 1);
        break;
    }
    if (lines_complete > 0)
    {
        _combo++;
        SetSoundPitch(_snd_clear, 1.f+(_combo*2.f));
        PlaySound(_snd_clear);
    } else {
        SetSoundPitch(_snd_clear, 1.f);
        _combo = 0;
    }

    // draw ghsot notrimino
    u16 notrimino = _notriminos[_mino][_rot];
    u8 oldy = _minoy;
    while (!check_offset(0, 1)) {
      _minoy++;
    }
    for (u8 i = 0; i < 16; i++) {
      u8 x = i % 4;
      u8 y = i / 4;
      u16 t = notrimino & (0x8000 >> i);
      if (t) {
        e2_putc(0xdb, NOTRIS_X + _minox + x, NOTRIS_Y + _minoy + y, 0x08);
        e2_effect_set(NOTRIS_X + _minox + x, NOTRIS_Y + _minoy + y,
                      EFFECT_RAINBOW);
      }
    }
    _minoy = oldy;

    // draw folling notrimino
    for (u8 i = 0; i < 16; i++)
    {
        u8 x = i % 4;
        u8 y = i / 4;
        u16 t = notrimino & (0x8000 >> i);
        if (t)
        {
            e2_putc(0xdb, NOTRIS_X + _minox + x, NOTRIS_Y + _minoy + y, _notri_color_map[_mino]);
        }

    }

    // draw hold and next
    e2_print("Hold", 1, 1, 0x0d);
    e2ext_box(1, 2, 5, 6, 0x07, 0x00);

    e2_print("Next", 18, 1, 0x0d);
    e2ext_box(18, 2, 5, 6, 0x07, 0x00);

    for (u8 i = 0; i < 16; i++)
    {
        u8 x = i % 4;
        u8 y = i / 4;
        u16 hold = _notriminos[_hold < 0 ? 0 : _hold][0];
        u16 t = hold & (0x8000 >> i);
        u16 next = _notriminos[_next][0];
        if (t && _hold > -1)
        {
            e2_putc(0xdb, 2+x, 3+y, _notri_color_map[_hold]);
        }
        t = next & (0x8000 >> i);
        if (t)
        {
            e2_putc(0xdb, 19+x, 3+y, _notri_color_map[_next]);
        }
    }


    if (_paused)
    {
        e2_rich_print("{0x0d}P{0x03}AUSED", NOTRIS_X + 2, 10);
    }

    if (_gameover)
    {
        e2_rich_print("{0x08}GAMEOVER", NOTRIS_X+1, 10);
    }
    
}


















