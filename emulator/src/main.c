#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern "C" {
    #include <raylib.h>

    #include <bootstrap.h>
    #include <e2.h>
    #include <g1.h>

    #include "chip8.h"
}

#define DECOMPILE(fmt, ...) (decompiled = (char*)FormatText(fmt, __VA_ARGS__))

#define lambda(lambda$_ret, lambda$_args, lambda$_body) \
    ({                                                  \
        lambda$_ret lambda$__anon$ lambda$_args         \
            lambda$_body& amp;                          \
        lambda$__anon$;                                 \
    })

static gui_layout_t* _gui;

static const Color _palette[16] = {
    (Color){0, 0, 0, 255},
    (Color){40, 42, 46, 255},
    (Color){55, 59, 65, 255},
    (Color){150, 152, 150, 255},
    (Color){180, 183, 180, 255},
    (Color){197, 200, 198, 255},
    (Color){234, 234, 234, 255},
    (Color){255, 255, 255, 255},
    (Color){213, 78, 83, 255},
    (Color){231, 140, 69, 255},
    (Color){231, 197, 71, 255},
    (Color){185, 202, 74, 255},
    (Color){112, 192, 177, 255},
    (Color){122, 166, 218, 255},
    (Color){195, 151, 216, 255},
    (Color){163, 104, 90, 255},
};

static void init();
static void update();
static void draw();

static Texture generate_bitmap_font(const char* font_file, u16 char_width, u16 char_height)
{

    Font font = LoadFont(font_file);
    RenderTexture rtex = LoadRenderTexture(char_width * 16, char_height * 16);

    // render bitmap font
    BeginTextureMode(rtex);
    for (u8 x = 0; x < 16; x++)
    {
        for (u8 y = 0; y < 16; y++)
        {
            u8 index = 16 * y + x;
            DrawTextEx(
                font,
                FormatText("%c", (x << 4) | y),
                (Vector2){x * char_width, y * char_height}, char_height, 1, WHITE);
        }
    }
    EndTextureMode();

    // flip texture
    RenderTexture flipped = LoadRenderTexture(char_width * 16, char_height * 16);
    BeginTextureMode(flipped);
    DrawTexture(rtex.texture, 0, 0, WHITE);
    EndTextureMode();

    // remove old buffer
    UnloadRenderTexture(rtex);

    // memory leak but w/e
    return flipped.texture;
}

int main(int argc, char** argv)
{
    bootstrap_window_t window = {
        .log_level = LOG_NONE,
        .window_flags = FLAG_WINDOW_RESIZABLE,
        .fps_max = 60,
        .exit_key = KEY_F12,
        .width = 95 * 10,
        .height = 56 * 10,
        .title = "SYNTAX",
    };

    bootstrap_e2_t e2 = {
        .width = window.width / 10,
        .height = window.height / 10,
        .update = update,
        .draw = draw,
    };

    bootstrap_init(&window, &e2);
    init();
    bootstrap_run();
    bootstrap_close();

    return 0;
}

// gui functions & variables
static bool _paused = true;

static void _btn_step()
{
    chip8_step();
}

static void _btn_continue()
{
    _paused = false;
}

static void _btn_pause()
{
    _paused = true;
}

// bootstrap callbacks
void init()
{

    // 0xDFF is the max size of a rom
    u8 rom[ROM_SIZE] = {};

    // load rom from file
    // TODO: only load size of the rom, not 0xDFF
    FILE* f = fopen("kp.c8", "rb");
    if (!f)
    {
        LOG("Couldn't open file!");
        exit(1);
    }
    fread(rom, ROM_SIZE, 1, f);
    fclose(f);

    // initialize chip8 emulator with rom data
    chip8_init(rom);

    // initialize main gui
    _gui = gui_create();

    gui_add_child(_gui, gui_button("Step", 1, 35, 0, 0x1d, _btn_step));
    gui_add_child(_gui, gui_button("Continue", 1, 36, 0, 0x1c, _btn_continue));
    gui_add_child(_gui, gui_button("Pause", 10, 36, 0, 0x18, _btn_pause));
}

void update()
{
}

void draw()
{
    // cursor position
    static e2vec2_t mpos;
    mpos = e2_get_mouse();

    // section borders, gui, & other info
    e2ext_box_legacy(0, 0, 65, 33, 0x07, 0x00, "Display");
    e2ext_box_legacy(66, 0, 28, 33, 0x07, 0x10, "Disassembler");
    e2ext_box_legacy(0, 34, 65, 21, 0x07, 0x10, "Tools");
    e2ext_box_legacy(66, 34, 28, 21, 0x07, 0x10, "Registers");

    gui_update(_gui);

    e2_rich_print(FormatText("{@17}_paused {@15}= %s", _paused ? "{@18}true" : "{@1c}false"), 1, 37, 0x07);

    // retrieve opcode info
    u8 high_byte = chip8_hb();
    u8 low_byte = chip8_lb();
    u8 bx = chip8_hb() & 0x0F;
    u8 by = (chip8_lb() & 0xF0) >> 4;
    u16 addr = ((u16)bx) << 8 | low_byte;

    // draw registers
    u8* regs = chip8_get_registers();
    for (u8 r = 0; r <= 0xF; r++)
    {
        e2_rich_print(FormatText("{@0d}_v{@05}[{@a}%1X{@05}] {@03}= {@0a}%04x", r, regs[r]), 68, 36 + r, 0x07);
    }

    e2_rich_print(FormatText("{@0d}_i  {@03}= {@04}0x{@0a}%04x", chip8_get_index()), 81, 36, 0x07);
    e2_rich_print(FormatText("{@0d}_pc {@03}= {@04}0x{@0a}%04x", chip8_get_pc()), 81, 37, 0x07);
    e2_rich_print(FormatText("{@0d}_op {@03}= {@04}0x{@0a}%04x", high_byte << 8 | low_byte), 81, 38, 0x07);

    // draw keypad
    // e2ext_box_legacy(82, 40, 8, 8, 0x07, 0x10, "Keypad");
    // static const u8 kstr[16] = {
    //     '1', '2', '3', '4',
    //     'Q', 'W', 'E', 'R',
    //     'A', 'S', 'D', 'F',
    //     'Z', 'X', 'C', 'V'};
    // const u8* kmap = chip8_get_keymap();

    // for (u8 k = 0; k < 16; k++)
    // {
    //     u8 kx = k % 4;
    //     u8 ky = k / 4;
    //     e2_rich_print(FormatText("{@%02x}%c", IsKeyDown(kmap[k]) ? 0xc1 : 0x15, kstr[k]), 83 + kx * 2, 41 + ky * 2, 0x07);
    // }

    // draw disassembler
    u8* rom = chip8_get_rom();
    static char* decompiled;

    for (u8 n = 0; n < 30; n++)
    {
        u8 xx = 73;
        u8 yy = 2 + n;

        u16 pc2 = fmax(chip8_get_pc() - 16, 0) + n * 2;
        if (pc2 > 0xFFF)
            continue;

        u8 color = 0x01;
        if (pc2 == chip8_get_pc())
            e2_putc('>', 67, yy, 0x08);

        high_byte = rom[pc2];
        low_byte = rom[pc2 + 1];
        bx = high_byte & 0x0F;
        by = (low_byte & 0xF0) >> 4;
        addr = ((u16)bx) << 8 | low_byte;
        u8 nibble = low_byte & 0x0F;

        DECOMPILE("{@04}%04x", high_byte << 8 | low_byte);

        switch (high_byte & 0xF0)
        {
        case 0x00:
        {
            switch (low_byte)
            {
            case 0xE0:
            {
                decompiled = "{@0d}CLS";
                break;
            }
            case 0xEE:
            {
                decompiled = "{@0e}RET";
                break;
            }
            case 0x00:
            {
                decompiled = "{@03}NOP";
                break;
            }
            }
            break;
        }
        case 0x10:
        {
            DECOMPILE("{@0d}JP {@03}addr{@04}0x{@0a}%03x", addr);
            break;
        }
        case 0x20:
        {
            DECOMPILE("{@0d}CALL {@03}addr{@04}0x{@0a}%03x", addr);
            break;
        }
        case 0x30:
        {
            DECOMPILE("{@0d}SE {@03}Vx{@04}0x{@0a}%1x {@03}byte{@04}0x{@0a}%02x", bx, low_byte);
            break;
        }
        case 0x40:
        {
            DECOMPILE("{@0d}SNE {@03}Vx{@04}0x{@0a}%1x, {@03}byte{@04}0x{@0a}%02x", bx, low_byte);
            break;
        }
        case 0x50:
        {
            DECOMPILE("{@0d}SE {@03}Vx{@04}0x{@0a}%1x {@03}Vy{@04}0x{@0a}%1x", bx, by);
            break;
        }
        case 0x60:
        {
            DECOMPILE("{@0d}LD {@03}Vx{@04}0x{@0a}%1x {@03}byte{@04}0x{@0a}%02x", bx, low_byte);
            break;
        }
        case 0x70:
        {
            DECOMPILE("{@0d}ADD {@03}Vx{@04}0x{@0a}%1x {@03}byte{@04}0x{@0a}%02x", bx, low_byte);
            break;
        }
        case 0x80:
        {
            switch (low_byte & 0x0F)
            {
            case 0x0:
            {
                DECOMPILE("{@0d}LD {@03}Vx{@04}0x{@0a}%1x {@03}Vy{@04}0x{@0a}%1x", bx, by);
                break;
            }
            case 0x1:
            {
                DECOMPILE("{@0d}OR {@03}Vx{@04}0x{@0a}%1x {@03}Vy{@04}0x{@0a}%1x", bx, by);
                break;
            }
            case 0x2:
            {
                DECOMPILE("{@0d}AND {@03}Vx{@04}0x{@0a}%1x {@03}Vy{@04}0x{@0a}%1x", bx, by);
                break;
            }
            case 0x3:
            {
                DECOMPILE("{@0d}XOR {@03}Vx{@04}0x{@0a}%1x {@03}Vy{@04}0x{@0a}%1x", bx, by);
                break;
            }
            case 0x4:
            {
                DECOMPILE("{@0d}ADD {@03}Vx{@04}0x{@0a}%1x {@03}Vy{@04}0x{@0a}%1x", bx, by);
                break;
            }
            case 0x5:
            {
                DECOMPILE("{@0d}SUB {@03}Vx{@04}0x{@0a}%1x {@03}Vy{@04}0x{@0a}%1x", bx, by);
                break;
            }
            case 0x6:
            {
                DECOMPILE("{@0d}SHR {@03}Vx{@04}0x{@0a}%1x {@03}Vy{@04}0x{@02}%1x", bx, by);
                break;
            }
            case 0x7:
            {
                DECOMPILE("{@0d}SUBN {@03}Vx{@04}0x{@0a}%1x {@03}Vy{@04}0x{@0a}%1x", bx, by);
                break;
            }
            case 0xE:
            {
                DECOMPILE("{@0d}SHL {@03}Vx{@04}0x{@0a}%1x {@03}Vy{@04}0x{@02}%1x", bx, by);
                break;
            }
            }
            break;
        }
        case 0x90:
        {
            DECOMPILE("{@0d}SNE {@03}Vx{@04}0x{@0a}%1x {@03}Vy{@04}0x{@0a}%1x", bx, by);
            break;
        }
        case 0xA0:
        {
            DECOMPILE("{@0d}LD {@0c}I {@03}addr{@04}0x{@0a}%03x", addr);
            break;
        }
        case 0xB0:
        {
            DECOMPILE("{@0d}JP {@0c}V0 {@03}addr{@04}0x{@0a}%03x", addr);
            break;
        }
        case 0xC0:
        {
            DECOMPILE("{@0d}RND {@03}Vx{@04}0x{@0a}%1x {@03}byte{@04}0x{@0a}%02x", bx, low_byte);
            break;
        }
        case 0xD0:
        {
            DECOMPILE("{@0d}DRW {@03}Vx{@04}0x{@0a}%1x {@03}Vy{@04}0x{@0a}%1x {@03}N{@04}0x{@0a}%1x", bx, by, nibble);
            break;
        }
        case 0xE0:
        {
            switch (low_byte)
            {
            case 0x9E:
                DECOMPILE("{@0d}SKP {@03}Vx{@04}0x{@0a}%1x", bx);
                break;
            case 0xA1:
                DECOMPILE("{@0d}SKNP {@03}Vx{@04}0x{@0a}%1x", bx);
                break;
            }
            break;
        }
        case 0xF0:
        {
            switch (low_byte)
            {
            case 0x07:
            {
                DECOMPILE("{@0d}LD {@03}Vx{@04}0x{@0a}%1x {@0c}DT", bx);
                break;
            }
            case 0x0A:
            {
                DECOMPILE("{@0d}LD {@03}Vx{@04}0x{@0a}%1x {@0c}K", bx);
                break;
            }
            case 0x15:
            {
                DECOMPILE("{@0d}LD {@0c}DT {@03}Vx{@04}0x{@0a}%1x", bx);
                break;
            }
            case 0x18:
            {
                DECOMPILE("{@0d}LD {@0c}ST {@03}Vx{@04}0x{@0a}%1x", bx);
                break;
            }
            case 0x1E:
            {
                DECOMPILE("{@0d}ADD {@0c}I {@03}Vx{@04}0x{@0a}%1x", bx);
                break;
            }
            case 0x29:
            {
                DECOMPILE("{@0d}LD {@0c}F {@03}Vx{@04}0x{@0a}%1x", bx);
                break;
            }
            case 0x33:
            {
                DECOMPILE("{@0d}LD {@0c}B {@03}Vx{@04}0x{@0a}%1x", bx);
                break;
            }
            case 0x55:
            {
                DECOMPILE("{@0d}LD {@0c}I {@03}Vx{@04}0x{@0a}%1x", bx);
                break;
            }
            case 0x65:
            {
                DECOMPILE("{@0d}LD {@03}Vx{@04}0x{@0a} {@0c}I%1x", bx);
                break;
            }
            }
            break;
        }
        }

        e2_rich_print(decompiled, xx, yy, color << 4);
        e2_print(FormatText("%03x", pc2), xx - 5, yy, pc2 == chip8_get_pc() ? 0x08 : 0x0e);
    }

    // step forward in chip8 rom
    if (!_paused)
        chip8_step();
}
