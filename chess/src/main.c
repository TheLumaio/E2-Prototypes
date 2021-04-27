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

#define CWHITE 0x70
#define CBLACK 0xc0

static Color _colors[16] = {
    (Color){0, 0, 0, 255},       // 0x0
    (Color){0, 40, 0, 255},      // 0x1
    (Color){0, 56, 0, 255},      // 0x2
    (Color){0, 88, 0, 255},      // 0x3
    (Color){0, 184, 0, 255},     // 0x4
    (Color){0, 216, 0, 255},     // 0x5
    (Color){0, 232, 0, 255},     // 0x6
    (Color){0, 248, 0, 255},     // 0x7
    (Color){255, 0, 0, 255},     // 0x8
    (Color){220, 150, 86, 255},  // 0x9
    (Color){247, 202, 136, 255}, // 0xa
    (Color){161, 181, 108, 255}, // 0xb
    (Color){121, 135, 79, 255},  // 0xc
    (Color){134, 193, 185, 255}, // 0xd
    (Color){124, 175, 194, 255}, // 0xe
    (Color){161, 105, 70, 255}   // 0xf
};

enum {
  EMPTY = 0x00,
  PAWN = 0x01,
  KNIGHT = 0x02,
  ROOK = 0x03,
  BISHOP = 0x04,
  KING = 0x05,
  QUEEN = 0x06,
  W_PAWN = 0x07,
  W_KNIGHT = 0x08,
  W_ROOK = 0x09,
  W_BISHOP = 0x0a,
  W_KING = 0x0b,
  W_QUEEN = 0x0c
};


static u8 _board[8][8];
static u8 _to_move = CWHITE;

static u8 _moves[16];
static u8 _num_moves = 0;
static u8 _selected = 0;
static bool _moving = false;

static u8 _pawn_moves = 0;
static u8 _check = EMPTY;

u8 get_piece(u8 x, u8 y) { return _board[x][y] & 0x0f; }

u8 get_color(u8 x, u8 y) { return _board[x][y] & 0xf0; }

void add_move(u8 x, u8 y) { _moves[_num_moves++] = y * 8 + x; }

bool add_if_valid(u8 x, u8 y)
{
    if (x<0||x>7||y<0||y>7) return true;
    if (get_piece(x, y) == EMPTY)
        add_move(x, y);
    else {
        u8 color = get_color(x, y);
        if (color == CBLACK && _to_move == CWHITE) add_move(x, y);
        if (color == CWHITE && _to_move == CBLACK) add_move(x, y);
        return true;
    }
    return false;
}

void get_moves()
{
    _num_moves = 0;

    u8 p_x = _selected%8;
    u8 p_y = _selected/8;
    u8 piece = _board[p_x][p_y]&0x0f;
    u8 color = _board[p_x][p_y]&0xf0;

    switch(piece)
    {
    case PAWN:
        if (_to_move == CWHITE)
        {
            if (get_piece(p_x, p_y-1) == EMPTY && !add_if_valid(p_x, p_y-1))
                if (get_piece(p_x, p_y-2) == EMPTY && p_y == 6) add_if_valid(p_x, p_y-2);
            if (get_piece(p_x-1, p_y-1) != EMPTY) add_if_valid(p_x-1, p_y-1);
            if (get_piece(p_x+1, p_y-1) != EMPTY) add_if_valid(p_x+1, p_y-1);
        }
        if (_to_move == CBLACK)
        {
            if (get_piece(p_x, p_y+1) == EMPTY && !add_if_valid(p_x, p_y+1))
                if (get_piece(p_x, p_y+2) == EMPTY && p_y == 1) add_if_valid(p_x, p_y+2);
            if (get_piece(p_x-1, p_y+1) != EMPTY) add_if_valid(p_x-1, p_y+1);
            if (get_piece(p_x+1, p_y+1) != EMPTY) add_if_valid(p_x+1, p_y+1);
        }

        break;
    case QUEEN: case ROOK:
        for (int x = p_x+1; x < 8; x++)
            if (add_if_valid(x, p_y)) break;
        for (int x = p_x-1; x >= 0; x--)
            if (add_if_valid(x, p_y)) break;
        for (int y = p_y+1; y < 8; y++)
            if (add_if_valid(p_x, y)) break;
        for (int y = p_y-1; y >= 0; y--)
            if (add_if_valid(p_x, y)) break;
        if (piece != QUEEN) break;
    case BISHOP:
      for (int x=0; x<8; x++)
          if (add_if_valid(p_x-x-1, p_y-x-1)) break;
      for (int x=0; x<8; x++)
          if (add_if_valid(p_x-x-1, p_y+x+1)) break;
      for (int x=0; x<8; x++)
          if (add_if_valid(p_x+x+1, p_y-x-1)) break;
      for (int x=0; x<8; x++)
          if (add_if_valid(p_x+x+1, p_y+x+1)) break;
      break;
    case KNIGHT:
      add_if_valid(p_x - 1, p_y - 2);
      add_if_valid(p_x + 1, p_y - 2);
      add_if_valid(p_x + 2, p_y - 1);
      add_if_valid(p_x + 2, p_y + 1);
      add_if_valid(p_x + 1, p_y + 2);
      add_if_valid(p_x - 1, p_y + 2);
      add_if_valid(p_x - 2, p_y + 1);
      add_if_valid(p_x - 2, p_y - 1);
      break;
    case KING:
        for (int x = 0; x < 3; x++)
            for (int y = 0; y < 3; y++)
                add_if_valid(p_x-1+x, p_y-1+y);
        break;
    default: break;
    }
    
}

int main(int argc, char** argv)
{

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(620, 480, "E2");
    SetTargetFPS(0);
    SetExitKey(KEY_F12);

    Image i = (Image){(void*)taffer, 160, 160, 1, 7};
    Texture texture = LoadTextureFromImage(i);

    e2_init(texture, GetScreenWidth()/10/3, GetScreenHeight()/10/3);

    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            _board[x][y] = EMPTY;
            if (y == 1 || y == 6)
                _board[x][y] = y==6?PAWN|CWHITE:PAWN|CBLACK;
            if (y == 0 || y == 7)
            {
                if (x == 0 || x == 7)
                    _board[x][y] = y==7?   ROOK|CWHITE : ROOK|CBLACK;
                if (x == 1 || x == 6)
                    _board[x][y] = y==7? KNIGHT|CWHITE : KNIGHT|CBLACK;
                if (x == 2 || x == 5)
                    _board[x][y] = y==7? BISHOP|CWHITE : BISHOP|CBLACK;
                if (x == 3)
                    _board[x][y] = y==7?  QUEEN|CWHITE : QUEEN|CBLACK;
                if (x == 4)
                    _board[x][y] = y==7?   KING|CWHITE : KING|CBLACK;
            }
        }
    }

    while (!WindowShouldClose())
    {
        e2_clear(0x0000);

        BeginDrawing();
        ClearBackground(BLACK);

        Vector2 mpos = e2_get_mouse();

        bool skip = false;
        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                u8 piece = _board[x][y]&0x0f;
                u8 color = (_board[x][y]&0xf0);

                switch(piece)
                {
                case PAWN:
                    piece = 'P'; break;
                case ROOK:
                    piece = 'R'; break;
                case KNIGHT:
                    piece = 'N'; break;
                case BISHOP:
                    piece = 'B'; break;
                case KING:
                    piece = 'K'; break;
                case QUEEN:
                    piece = 'Q'; break;
                }
                u8 ncolor = color;
                if (x%2==(y%2==0)?1:0)
                    ncolor = color>>4|0x10;
                else
                    ncolor = color>>4|0x20;

                if (_moving) {
                    for (int i = 0; i < _num_moves; i++) {
                        u8 move = _moves[i];
                        u8 m_x = move % 8;
                        u8 m_y = move / 8;
                        if (x == m_x && y == m_y && _moving)
                            ncolor = color>>4 | 0xd0;
                        if (mpos.x == m_x+6 && mpos.y == m_y+3)
                        {
                            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                            {
                                u8 s_x = _selected%8;
                                u8 s_y = _selected/8;
                                _board[m_x][m_y] = _board[s_x][s_y];
                                _board[s_x][s_y] = EMPTY;
                                _moving = false;
                                skip = true;
                                _to_move = CBLACK;
                            }
                        }
                    }
                }

                if (!skip && mpos.x == x + 6 && mpos.y == y + 3) {
                    if (color == CWHITE && _moving)
                        ncolor = 0x99;
                    if (color == CWHITE && piece != EMPTY && _to_move == CWHITE
                        && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        _moving = true;
                        _selected = y * 8 + x;
                        get_moves();
                    }
                }

                if (mpos.x == x + 6 && mpos.y == y + 3)
                    ncolor = color >> 4 | 0x80;

                e2_putc(piece, x+6, y+3, ncolor);
            }
        }

        if (!_moving) {
            for (int i = 0; i < 16; i++) {
                u8 s_x = i % 8;
                u8 s_y = i / 8;
                u8 c1 = get_color(s_x, s_y);
                u8 c2 = _to_move;
                if (c1 == c2) {
                    _selected = i;
                    get_moves();
                    for (int j = 0; j < _num_moves; j++) {
                        u8 m_x = _moves[j] % 8;
                        u8 m_y = _moves[j] / 8;
                        if (get_piece(m_x, m_y) == KING)
                            _check = _to_move;
                    }
                }
            }
        }
        
        if (_to_move == CBLACK && !_check) {
            u8 opp_pieces[16];
            u8 num_pieces = 0;
            for (u8 x = 0; x < 8; x++) {
                for (u8 y = 0; y < 8; y++) {
                    if (get_color(x, y) == CBLACK) {
                        opp_pieces[num_pieces++] = y * 8 + x;
                    }
                }
            }
        SELECT:
            _selected = rand()%num_pieces;
            get_moves();
            if (_num_moves == 0)
                goto SELECT;
            // ^^^ hacky as fuck don't do that
            u8 opp_move = rand()%_num_moves;
            
            u8 s_x = _selected%8;
            u8 s_y = _selected/8;
            u8 m_x = _moves[opp_move]%8;
            u8 m_y = _moves[opp_move]/8;
            e2_putc('#', 6+s_x, 3+s_y, 0xff);
            _board[m_x][m_y] = _board[s_x][s_y];
            _board[s_x][s_y] = EMPTY;
            _to_move = CWHITE;
        }
        
        if (_check)
        {
            const char* text = FormatText("%s in {0x08}check", _to_move==CWHITE?"You're":"Enemy is");
            e2_rich_print(text, 0, 2);
        }
        
        
        e2_render();

        EndDrawing();
    }

    e2_close();

    CloseWindow();

    return 0;

}
