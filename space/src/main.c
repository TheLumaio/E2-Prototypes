#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

#include <raylib.h>

#include "e2.h"
#include "g1.h"
#include "bootstrap.h"
#include "str.h"

#include "world.h"

#include "entity.h"

#define LOG_WINDOW_HEIGHT 15

static gui_layout_t* _layout = NULL;
static gui_layout_t* _menu_bar = NULL;

static float _world_tick = 0;
static const float _world_speed = 0.2;

static const u8 _mask_lookup[16] =
{
    0x09,// 0b0000 none
    0xb3,// 0b1000 above
    0xc4,// 0b0100 right
    0xc0,// 0b1100 above right
    0xb3,// 0b0010 down
    0xb3,// 0b1010 down above
    0xda,// 0b0110 right down
    0xc3,// 0b1110 up right down
    0xc4,// 0b0001 left
    0xd9,// 0b1001 above left
    0xc4,// 0b0101 right left
    0xc1,// 0b1101 above right left
    0xbf,// 0b0011 down left
    0xb4,// 0b1011 above down left
    0xc2,// 0b0111 right down left
    0xc5 // 0b1111 above right down left

}; 

static struct {
    i16 x, y, w, h;
    i16 off_x, off_y;
} _camera;

static struct {
    i16 local_x;
    i16 local_y;
    i16 chunk_x;
    i16 chunk_y;
} _player;

static struct {
    list_t* log;
    u16 scroll;
} _log;

enum {
  MODE_MOVE,
  MODE_INTERACT,
  MODE_PICKUP
};

static int _mode = MODE_MOVE;

static void init();
static void update();
static void draw();

static int distance(int x1, int y1, int x2, int y2)
{
  int dx = x2 - x1;
  int dy = y2 - y1;
  return sqrt(dx * dx + dy * dy);
}

static void log_entry(const char* log)
{
    str_t* str = str_from_cstr(log);
    list_append(_log.log, str);
    if (_log.log->entries-_log.scroll >= LOG_WINDOW_HEIGHT)
    {
        _log.scroll++;
    }
}

static void move_player(i8 ox, i8 oy)
{
    _player.local_x += ox;
    _player.local_y += oy;

    // x move
    if (_player.local_x < 0)
    {
        _player.local_x += 16;
        _player.chunk_x--;
    }
    if (_player.local_x > 15)
    {
        _player.local_x -= 16;
        _player.chunk_x++;
    }
    // y move
    if (_player.local_y < 0)
    {
        _player.local_y += 16;
        _player.chunk_y--;
    }
    if (_player.local_y > 15)
    {
        _player.local_y -= 16;
        _player.chunk_y++;
    }
}

static u8 get_bit_mask(chunk_t* chunk, u16 x, u16 y)
{
    int mask_array[4];
    memset(mask_array, 0, sizeof(mask_array));

    e2vec2_t neighbors[4] = {
        {x-0, y-1},
        {x+1, y-0},
        {x-0, y+1},
        {x-1, y-0},
    };

    for (int i = 0; i < 4; i++)
    {
        e2vec2_t current_pos = neighbors[i];
        chunk_t* node_chunk = chunk;
        if (current_pos.x < 0)
        {
            current_pos.x += WORLD_CHUNK_SIZE;
            node_chunk = world_get_chunk(node_chunk->x-1, node_chunk->y);
        }
        else if (current_pos.x >= WORLD_CHUNK_SIZE)
        {
            current_pos.x -= WORLD_CHUNK_SIZE;
            node_chunk = world_get_chunk(node_chunk->x+1, node_chunk->y);
        }

        if (!node_chunk) {
            mask_array[i] = 0;
            continue;
        }
        
        if (current_pos.y < 0)
        {
            current_pos.y += WORLD_CHUNK_SIZE;
            node_chunk = world_get_chunk(node_chunk->x, node_chunk->y-1);
        }
        else if (current_pos.y >= WORLD_CHUNK_SIZE)
        {
            current_pos.y -= WORLD_CHUNK_SIZE;
            node_chunk = world_get_chunk(node_chunk->x, node_chunk->y+1);
        }

        if (!node_chunk) {
            mask_array[i] = 0;
            continue;
        }

        u8 color = node_chunk->data[current_pos.y*WORLD_CHUNK_SIZE+current_pos.x]>>8;
        if (color == 0x01) { continue; }
        if (chunk_get_tile(node_chunk, current_pos.x, current_pos.y) == '#' && color == 0x03)
            mask_array[i] = 0b1;
    }


    u8 mask = 0b0000;
    for (int i = 0; i < 4; i++)
        mask |= mask_array[i]<<(i);

    return mask;
}

int main(int argc, char** argv)
{
    // do bootstrap
    bootstrap_window_t window = {
        .log_level = LOG_NONE,
        .window_flags = FLAG_WINDOW_RESIZABLE,
        .fps_max = 60,
        .exit_key = KEY_F12,
        .width = 640,
        .height = 640,
        .title = "SPACE"
    };

    bootstrap_e2_t e2 = {
        .width = window.width/10,
        .height = window.height/10,
        .update = update,
        .draw = draw
    };

    bootstrap_init(&window, &e2);

    // do init
    init();

    // do run
    bootstrap_run(); 
    bootstrap_close();

    return 0;
}

void init()
{
    _menu_bar = gui_create();
    gui_add_child(_menu_bar, gui_button("FILE", 0, 0, 0, 0x27, NULL));
    _layout = _menu_bar;
    
    world_load("station.world");

    _camera.x = 1;
    _camera.y = 2;
    _camera.w = 45;
    _camera.h = 45;

    _camera.off_x = 0;
    _camera.off_y = 0;

    _player.local_x = 0;
    _player.local_y = 0;
    _player.chunk_x = 0;
    _player.chunk_y = 0;

    _log.log = list_create(8);
    _log.scroll = 0;
}

void update()
{
    int k = 0;
    while ((k = GetKeyPressed()) != 0)
    {
        switch(k)
        {
            // movement/interaction keys
        case KEY_LEFT:
            switch(_mode)
            {
            case MODE_MOVE:
                move_player(-1, 0);
                break;
            case MODE_INTERACT: {
                move_player(-1, 0);
                chunk_t* chunk = world_get_chunk(_player.chunk_x, _player.chunk_y);
                ent_base_t* fire = ent_fire(chunk, _player.local_x, _player.local_y, 3);
                chunk_add_ent(chunk, fire);
                move_player(1, 0);
                _mode = MODE_MOVE;
            } break;
            }
            break;
        case KEY_RIGHT:
            move_player(1, 0);
            break;
        case KEY_UP:
            move_player(0, -1);
            break;
        case KEY_DOWN:
            move_player(0, 1);
            break;

            // mode selection
        case KEY_SPACE:
            if (_mode == MODE_INTERACT)
            {
                // TODO: use item;
            }
            else if (_mode == MODE_MOVE)
            {
                log_entry("{0x0d}Mode switch: {0x07}INTERACT");
                _mode = MODE_INTERACT;
            }
            break;
        case KEY_G:
            if (_mode == MODE_PICKUP)
            {
                // TODO: pickup item
            }
            else if (_mode == MODE_MOVE)
            {
                _mode = MODE_PICKUP;
            }
            break;
        case KEY_ESCAPE:
            if (_mode != MODE_MOVE)
            {
                log_entry("{0x0d}Mode swtch: {0x07}MOVE");
                _mode = MODE_MOVE;
            }
            break;
        default:
            break;
        }
    }

    // TODO: do entity update
    // TODO: scrap all of this garbage, instead
    // use a 16x16 array just like the tiles
    _world_tick += GetFrameTime();
    if (_world_tick < _world_speed) return;
    _world_tick = 0;

    world_t* world = world_get();
    for (int i = world->to_update->entries-1; i >= 0; i--)
    {
        chunk_t* chunk = world->to_update->data[i];
        for (int x = 0; x < WORLD_CHUNK_SIZE; x++)
        {
            for (int y = 0; y < WORLD_CHUNK_SIZE; y++)
            {
                ent_base_t* ent = chunk->ents[y*WORLD_CHUNK_SIZE+x];
                if (ent != NULL)
                    ent->ptr(ent);
            }
        }
    }
}

void draw()
{
    // do ui stuff
    e2_print(FormatText("%64s", FormatText("%d", GetFPS())), 0, 0, 0x27);
    gui_update(_layout);

    _camera.off_x = (_player.local_x+(_player.chunk_x*16))-_camera.w/2;
    _camera.off_y = (_player.local_y+(_player.chunk_y*16))-_camera.h/2;

    e2ext_box(0, 1, 47, 47, 0x07, 0x00);
    e2ext_box(47, 1, 17, 47, 0x07, 0x00);
    e2ext_box(0, 48, 64, 16, 0x07, 0x00);
    e2_print("World", 2, 1, 0x07);
    e2_print("Stats", 49, 1, 0x07);
    e2_print("Log", 2, 48, 0x07);

    // do world draw
    for (u16 x = 0; x < _camera.w; x++)
    {
        for (u16 y = 0; y < _camera.h; y++)
        {
            i16 chunk_x = (_camera.off_x+x)>>4;
            i16 chunk_y = (_camera.off_y+y)>>4;
            chunk_t* chunk = world_get_chunk(chunk_x, chunk_y);
            if (!chunk) { continue; }

            int world_x = distance(_camera.off_x+x, 0, chunk_x*16, 0);
            int world_y = distance(0, _camera.off_y+y, 0, chunk_y*16);

            u8 tile = chunk_get_tile(chunk, world_x, world_y);
            u8 color = chunk->data[world_y*16+world_x]>>8;

            if (world_x == _player.local_x && world_y == _player.local_y &&
                chunk_x == _player.chunk_x && chunk_y == _player.chunk_y)
            {
                tile = '@';
                color = 0x0d;
            }

            if (tile == '#' && color == 0x03)
            {
                // + get neighbors
                // + create bit mask from neighbors
                // + pick new tile from mask array
                u8 mask = get_bit_mask(chunk, world_x, world_y);
                tile = _mask_lookup[mask];
            }

            /* TODO: do entity draw
            for (int i = 0; i < chunk->ents->entries; i++)
            {
                ent_base_t* ent = chunk->ents->data[i];
                if (ent->x == world_x && ent->y == world_y)
                {
                    tile = ent->tile;
                    color = ent->color;
                }
                }*/
            ent_base_t* ent = chunk->ents[world_y*WORLD_CHUNK_SIZE+world_x];
            if (ent != NULL)
            {
                tile = ent->tile;
                color = ent->color;
            }

            Vector2 mpos = e2_get_mouse();
            if (world_x+(chunk_x*16) == mpos.x+_camera.off_x-1 && world_y+(chunk_y*16) == mpos.y+_camera.off_y-2)
            {
                tile = 0xdb;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    LOG("%d", world_x, world_y);
                    if (chunk->ents[world_y*WORLD_CHUNK_SIZE+world_x] == NULL)
                        chunk_add_ent(chunk, ent_fire(chunk, world_x, world_y, 3));
                }
            }

            if (!chunk) continue;
            e2_putc(tile, _camera.x+x, _camera.y+y, color);
            
        }
    }

    // do log draw
    for (int i = _log.scroll; i < _log.log->entries; i++)
    {
        str_t* str = _log.log->data[i];
        e2_rich_print((char*)str->data, 1, 49+i-_log.scroll);
    }
    
}


