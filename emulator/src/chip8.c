#include "chip8.h"

#define Vx _v[bx]
#define Vy _v[by]
#define V(a) _v[0x##a]
#define VV(a) _v[a]

static u8 _ram[0xFFF] = {
	// 4x5 low-res mode font sprites (0-F)
	0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20,
	0x20, 0x70, 0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0,
	0x10, 0xF0, 0x10, 0xF0, 0xA0, 0xA0, 0xF0, 0x20,
	0x20, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80,
	0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40,
	0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0,
	0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0,
	0x90, 0xE0, 0x90, 0xE0, 0xF0, 0x80, 0x80, 0x80,
	0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0xF0, 0x80,
	0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80,

	// 8x10 high-res mode font sprites (0-F)
	0x3C, 0x7E, 0xE7, 0xC3, 0xC3, 0xC3, 0xC3, 0xE7,
	0x7E, 0x3C, 0x18, 0x38, 0x58, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x3C, 0x3E, 0x7F, 0xC3, 0x06,
	0x0C, 0x18, 0x30, 0x60, 0xFF, 0xFF, 0x3C, 0x7E,
	0xC3, 0x03, 0x0E, 0x0E, 0x03, 0xC3, 0x7E, 0x3C,
	0x06, 0x0E, 0x1E, 0x36, 0x66, 0xC6, 0xFF, 0xFF,
	0x06, 0x06, 0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFE,
	0x03, 0xC3, 0x7E, 0x3C, 0x3E, 0x7C, 0xC0, 0xC0,
	0xFC, 0xFE, 0xC3, 0xC3, 0x7E, 0x3C, 0xFF, 0xFF,
	0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x60,
	0x3C, 0x7E, 0xC3, 0xC3, 0x7E, 0x7E, 0xC3, 0xC3,
	0x7E, 0x3C, 0x3C, 0x7E, 0xC3, 0xC3, 0x7F, 0x3F,
	0x03, 0x03, 0x3E, 0x7C, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	// 6-bit ASCII character patterns
	0x00, // |        |
	0x10, // |   *    |
	0x20, // |  *     |
	0x88, // |*   *   |
	0xA8, // |* * *   |
	0x50, // | * *    |
	0xF8, // |*****   |
	0x70, // | ***    |
	0x80, // |*       |
	0x90, // |*  *    |
	0xA0, // |* *     |
	0xB0, // |* **    |
	0xC0, // |**      |
	0xD0, // |** *    |
	0xE0, // |***     |
	0xF0, // |****    |

	// 6-bit ASCII characters from 0x100-
	0x46, 0x3E, 0x56, // @
	0x99, 0x9F, 0x4F, // A
	0x5F, 0x57, 0x4F, // B
	0x8F, 0x88, 0x4F, // C
	0x5F, 0x55, 0x4F, // D
	0x8F, 0x8F, 0x4F, // E
	0x88, 0x8F, 0x4F, // F
	0x9F, 0x8B, 0x4F, // G
	0x99, 0x9F, 0x49, // H
	0x27, 0x22, 0x47, // I
	0xAE, 0x22, 0x47, // J
	0xA9, 0xAC, 0x49, // K
	0x8F, 0x88, 0x48, // L
	0x43, 0x64, 0x53, // M
	0x99, 0xDB, 0x49, // N
	0x9F, 0x99, 0x4F, // O
	0x88, 0x9F, 0x4F, // P
	0x9F, 0x9B, 0x4F, // Q
	0xA9, 0x9F, 0x4F, // R
	0x1F, 0x8F, 0x4F, // S
	0x22, 0x22, 0x56, // T
	0x9F, 0x99, 0x49, // U
	0x22, 0x55, 0x53, // V
	0x55, 0x44, 0x54, // W
	0x53, 0x52, 0x53, // X
	0x22, 0x52, 0x53, // Y
	0xCF, 0x12, 0x4F, // Z
	0x8C, 0x88, 0x3C, // [
	0x10, 0xC2, 0x40, // \
	0x2E, 0x22, 0x3E, // ]
	0x30, 0x25, 0x50, // ^
	0x06, 0x00, 0x50, // _
	0x00, 0x00, 0x40, // space
	0x0C, 0xCC, 0x2C, // !
	0x00, 0x50, 0x45, // "
	0x65, 0x65, 0x55, // #
	0x46, 0x46, 0x56, // $
	0xDF, 0xBF, 0x4F, // %
	0x5F, 0xAF, 0x4E, // &
	0x00, 0x80, 0x18, // '
	0x21, 0x22, 0x41, // (
	0x12, 0x11, 0x42, // )
	0x53, 0x56, 0x53, // *
	0x22, 0x26, 0x52, // +
	0x2E, 0x00, 0x30, // ,
	0x00, 0x06, 0x50, // -
	0xCC, 0x00, 0x20, // .
	0xC0, 0x12, 0x40, // /
	0x9F, 0x99, 0x4F, // 0
	0x22, 0x22, 0x32, // 1
	0x8F, 0x1F, 0x4F, // 2
	0x1F, 0x1F, 0x4F, // 3
	0x22, 0xAF, 0x4A, // 4
	0x1F, 0x8F, 0x4F, // 5
	0x9F, 0x8F, 0x4F, // 6
	0x11, 0x11, 0x4F, // 7
	0x9F, 0x9F, 0x4F, // 8
	0x1F, 0x9F, 0x4F, // 9
	0x80, 0x80, 0x10, // :
	0x2E, 0x20, 0x30, // ;
	0x21, 0x2C, 0x41, // <
	0xE0, 0xE0, 0x30, // =
	0x2C, 0x21, 0x4C, // >
	0x88, 0x1F, 0x4F, // ?

	// Extra scratch memory in the ROM, used for ASCII sprites.
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static u8 _v[16];

static u16 _pc = 0x000;
static u16 _i = 0x0000;

static u8 _dt = 0x00;
static u8 _st = 0x00;

static u16 _stack[16];
static u8 _sp = 0;

static u8 _keypad[16];

static const u8 _keymap[16] = {
    KEY_ONE,   // 0x01
    KEY_TWO,   // 0x02
    KEY_THREE, // 0x03
    KEY_FOUR,  // 0x0C
    KEY_Q,     // 0x04
    KEY_W,     // 0x05
    KEY_E,     // 0x06
    KEY_R,     // 0x0D
    KEY_A,     // 0x07
    KEY_S,     // 0x08
    KEY_D,     // 0x09
    KEY_F,     // 0x0E
    KEY_Z,     // 0x0A
    KEY_X,     // 0x00
    KEY_C,     // 0x0B
    KEY_V      // 0x0F
};

void hexDump(const char* desc, const void* addr, const int len, const int pL)
{
    // Silently ignore silly per-line values.
    int perLine = pL;
    if (perLine < 4 || perLine > 64)
        perLine = 16;

    int i;
    unsigned char buff[perLine + 1];
    const unsigned char* pc = (const unsigned char*)addr;

    // Output description if given.

    if (desc != NULL)
        printf("%s:\n", desc);

    // Length checks.

    if (len == 0)
    {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0)
    {
        printf("  NEGATIVE LENGTH: %d\n", len);
        return;
    }

    // Process every byte in the data.

    for (i = 0; i < len; i++)
    {
        // Multiple of perLine means new or first line (with line offset).

        if ((i % perLine) == 0)
        {
            // Only print previous-line ASCII buffer for lines beyond first.

            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset of current line.

            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.

        printf(" %02x", pc[i]);

        // And buffer a printable ASCII character for later.

        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) // isprint() may be better.
            buff[i % perLine] = '.';
        else
            buff[i % perLine] = pc[i];
        buff[(i % perLine) + 1] = '\0';
    }

    // Pad out last line if not exactly perLine characters.

    while ((i % perLine) != 0)
    {
        printf("   ");
        i++;
    }

    // And print the final ASCII buffer.

    printf("  %s\n", buff);
}

void chip8_init(u8* rom)
{
    memcpy(&_ram[0x200], rom, sizeof(u8) * ROM_SIZE);
    _pc = 0x200;

    memset(_v, 0x00, sizeof(u8) * 16);
}

void chip8_step()
{
    u8 high_byte = _ram[_pc];
    u8 low_byte = _ram[_pc + 1];
    u8 bx = high_byte & 0x0F;
    u8 by = (low_byte & 0xF0) >> 4;
    u8 nibble = low_byte & 0x0F;
    u16 addr = ((u16)bx) << 8 | low_byte;

    switch (high_byte & 0xF0)
    {
    case 0x00:
    {
        if (bx == 0)
        {
            switch (low_byte)
            {
            case 0xE0:
            {
                e2_clear(0);
                break;
            }
            case 0xEE:
            {
                if (_sp > 0)
                {
                    _pc = _stack[_sp - 1];
                    _sp--;
                }
                break;
            }
            }
        }
        break;
    }
    case 0x10:
    {
        _pc = addr - 2;
        break;
    }
    case 0x20:
    {
        _stack[_sp++] = _pc;
        _pc = addr - 2;
        break;
    }
    case 0x30:
    {
        if (Vx == low_byte)
        {
            _pc += 2;
        }
        break;
    }
    case 0x40:
    {
        if (Vx != low_byte)
        {
            _pc += 2;
        }
        break;
    }
    case 0x50:
    {
        if (Vx == Vy)
        {
            _pc += 2;
        }
        break;
    }
    case 0x60:
    {
        Vx = low_byte;
        break;
    }
    case 0x70:
    {
        Vx += low_byte;
        break;
    }
    case 0x80:
    {
        switch (low_byte & 0x0F)
        {
        case 0x0:
        {
            Vx = Vy;
            break;
        }
        case 0x1:
        {
            Vx = Vx | Vy;
            break;
        }
        case 0x2:
        {
            Vx = Vx & Vy;
            break;
        }
        case 0x3:
        {
            Vx = Vx ^ Vy;
            break;
        }
        case 0x4:
        {
            Vx = Vx + Vy;
            if (Vx + Vy > 255)
                V(F) = 1;
            else
                V(F) = 0;
            break;
        }
        case 0x5:
        {
            if (Vx > Vy)
                V(F) = 1;
            else
                V(F) = 0;
            Vx = Vx - Vy;
            break;
        }
        case 0x6:
        {
            V(F) = (Vx & 0x1);
            Vx = Vx >> 1;
            break;
        }
        case 0x7:
        {
            if (Vy > Vx)
                V(F) = 1;
            else
                V(F) = 0;
            Vx = Vy - Vx;
            break;
        }
        case 0xE:
        {
            V(F) = (Vx & 0x80) >> 0x7;
            Vx = Vx << 1;
            break;
        }
        }
        break;
    }
    case 0x90:
    {
        if (Vx != Vy)
            _pc += 2;
        break;
    }
    case 0xA0:
    {
        _i = addr;
        break;
    }
    case 0xB0:
    {
        _pc = addr + V(0) - 2;
        break;
    }
    case 0xC0:
    {
        Vx = (rand() % 256) & low_byte;
        break;
    }
    case 0xD0:
    {
        byte_info_t info; // = e2_get_byte_info(Vx, Vy);
        V(F) = 0;

        for (u16 y = 0; y < nibble; y++)
        {
            u8 byte = _ram[_i + y];

            for (u8 x = 0; x < 8; x++)
            {
                u8 bit = byte & (0x80 >> x);
                info = e2_get_byte_info(Vx + x + 1, Vy + y + 1);
                if (bit)
                {
                    if (info.fg == 0xFF)
                    {
                        V(F) = 1;
                    }

                    e2_putc(0xdb, Vx + x + 1, Vy + y + 1, info.fg ^ 0xFF);
                }
            }
        }

        break;
    }
    case 0xE0:
    {
        switch (low_byte)
        {
        case 0x9E:
            if (IsKeyDown(_keymap[Vx]))
                _pc += 2;
            break;
        case 0xA1:
            if (IsKeyUp(_keymap[Vx]))
                _pc += 2;
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
            Vx = _dt;
            break;
        }
        case 0x0A:
        {
            _pc -= 2;
            u8 key = GetKeyPressed();
            for (u8 i = 0; i < 16; i++)
            {
                if (_keymap[i] == key)
                {
                    Vx = i;
                    _pc += 2;
                    break;
                }
            }

            break;
        }
        case 0x15:
        {
            _dt = Vx;
            break;
        }
        case 0x18:
        {
            _st = Vx;
            break;
        }
        case 0x1E:
        {
            _i += Vx;
            break;
        }
        case 0x29:
        {
            _i = 5 * Vx;
            break;
        }
        case 0x33:
        {
            u8 value = Vx;

            _ram[_i + 2] = value % 10;
            value /= 10;

            _ram[_i + 1] = value % 10;
            value /= 10;

            _ram[_i + 0] = value % 10;
            break;
        }
        case 0x55:
        {
            for (u8 reg = 0; reg <= Vx; reg++)
                _ram[_i + reg] = VV(reg);
            break;
        }
        case 0x65:
        {
            for (u8 reg = 0; reg <= Vx; reg++)
                VV(reg) = _ram[_i + reg];
            break;
        }
        }
        break;
    }
    }

    // incrememt program counter at the very end
    _pc += 2;
}

u8 chip8_hb() { return _ram[_pc]; }
u8 chip8_lb() { return _ram[_pc + 1]; }

void chip8_set_key(u8 key, bool pressed) { _keypad[key] = pressed; }

u16 chip8_get_index()
{
    return _i;
}

u8* chip8_get_registers()
{
    return _v;
}

u8 chip8_get_delay()
{
    return _dt;
}

u8 chip8_get_sound()
{
    return _st;
}

u16 chip8_get_pc()
{
    return _pc;
}

u8* chip8_get_rom()
{
    return _ram;
}

const u8* chip8_get_keymap()
{
    return _keymap;
}
