#ifndef CHIP_H
#define CHIP_H

#include <e2.h>

#define ROM_SIZE 0xDFF

enum CHIP8_KEYPAD_KEY
{
    KEYPAD_1,
    KEYPAD_2,
    KEYPAD_3,
    KEYPAD_C,
    KEYPAD_4,
    KEYPAD_5,
    KEYPAD_6,
    KEYPAD_D,
    KEYPAD_7,
    KEYPAD_8,
    KEYPAD_9,
    KEYPAD_E,
    KEYPAD_A,
    KEYPAD_0,
    KEYPAD_B,
    KEYPAD_F,
};

void chip8_init(u8* rom);
void chip8_step();
u8 chip8_hb();
u8 chip8_lb();
void chip8_set_key(u8 key, bool pressed);
u16 chip8_get_index();
u8* chip8_get_registers();
u8 chip8_get_delay();
u8 chip8_get_sound();
u16 chip8_get_pc();
u8* chip8_get_rom();
const u8* chip8_get_keymap();

#endif
