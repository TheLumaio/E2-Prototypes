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

#include "taffer.h"

#include "e2.h"
#include "g2.h"

#define H 0x03
#define D 0x04
#define C 0x05
#define S 0x06

typedef struct card_t card_t;
struct card_t {
    uint8_t s;
    uint8_t n;
    bool face;
    card_t* child;
    card_t* parent;
};

#define stack_t card_t

static stack_t* deck = NULL;
static card_t* tail = NULL;
static stack_t* flip = NULL;

static stack_t* stacks[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static card_t* h_stack = NULL;
static card_t* d_stack = NULL;
static card_t* c_stack = NULL;
static card_t* s_stack = NULL;

static bool dragging = false;
static int8_t drag_from = -1;
static card_t* mouse_card = NULL;

static const char* card_back =
"{0x07}\xc9\xcd\xcd\xcd\xcd\xcd\xbb\n"
"\xba#####\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#####\xba\n"
"\xc8\xcd\xcd\xcd\xcd\xcd\xbc";

static const char* card_H =
"{0x03}\xc9\xcd\xcd\xcd\xcd\xcd\xbb\n"
"\xba#####\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba# \x03 #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#####\xba\n"
"\xc8\xcd\xcd\xcd\xcd\xcd\xbc";

static const char* card_D =
"{0x03}\xc9\xcd\xcd\xcd\xcd\xcd\xbb\n"
"\xba#####\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba# \x04 #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#####\xba\n"
"\xc8\xcd\xcd\xcd\xcd\xcd\xbc";

static const char* card_C =
"{0x03}\xc9\xcd\xcd\xcd\xcd\xcd\xbb\n"
"\xba#####\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba# \x05 #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#####\xba\n"
"\xc8\xcd\xcd\xcd\xcd\xcd\xbc";

static const char* card_S =
"{0x03}\xc9\xcd\xcd\xcd\xcd\xcd\xbb\n"
"\xba#####\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba# \x06 #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#   #\xba\n"
"\xba#####\xba\n"
"\xc8\xcd\xcd\xcd\xcd\xcd\xbc";

static char* get_card(card_t* c)
{
    const char* card =
    "{0x%d7}\xc9\xcd\xcd\xcd\xcd\xcd\xbb\n"
    "\xba%c %c  \xba\n"
    "\xba     \xba\n"
    "\xba     \xba\n"
    "\xba     \xba\n"
    "\xba  %c  \xba\n"
    "\xba     \xba\n"
    "\xba     \xba\n"
    "\xba     \xba\n"
    "\xba  %c %c\xba\n"
    "\xc8\xcd\xcd\xcd\xcd\xcd\xbc";

    uint8_t ca = 0;
    uint8_t cb = 0;
    switch(c->s)
    {
        case S:;case C: ca=1; break;
        case H:;case D: ca=8; break;
    }

    uint8_t n = '0';
    if (c->n == 1) n = 'A';
    else if (c->n == 10) n = 'T';
    else if (c->n == 11) n = 'J';
    else if (c->n == 12) n = 'Q';
    else if (c->n == 13) n = 'K';
    else n = '0' + c->n;

    return (char*)FormatText(card, ca, c->s, n, n, n, c->s);
}

static void deck_shuffle()
{
    card_t* list[52];
    card_t* card = deck;
    for (int i = 0; i < 52; i++)
    {
        list[i] = card;
        card = card->child;
    }

    srand(time(NULL));
    for (int i = 0; i < 52; i++)
    {
        int r = i + (rand()%(52-i));

        card_t* a = list[i];
        card_t* b = list[r];

        list[i] = b;
        list[r] = a;
    }

    deck = list[0];
    card = deck;
    deck->parent = NULL;
    for (int i = 1; i < 52-1; i++)
    {
        list[i]->parent = list[i-1];
        list[i]->child = list[i+1];

        list[i-1]->child = list[i];
        list[i+1]->parent = list[i];
    }

    tail = list[51];
    tail->child = NULL;
    deck->parent = NULL;
}

static bool clicked = false;
static bool click_box(int x, int y, int w, int h)
{
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return false;
    Vector2 m = e2_get_mouse();
    if (m.x>=x && m.x<x+w && m.y>=y && m.y<y+h)
    {
        clicked = true;
        return true;
    }
    return false;
}


// can <a> stack on top of <b>
static bool can_stack(card_t* a, card_t* b)
{
    bool can = true;
    if ((a->s == H || a->s == D) && (b->s == H || b->s == D)) can = false;
    if ((a->s == C || a->s == S) && (b->s == C || b->s == S)) can = false;
    if (a->n != b->n-1) can = false;
    return can;
}

static void flip_deck()
{
    if (flip == NULL)
    {
        flip = deck;
    }
    else if (flip == tail)
    {
        flip = NULL;
    }
    else
    {
        flip = flip->child;
    }

}

static void init_game()
{
    // initialize new game
    deck = malloc(sizeof(stack_t));
    deck->child = NULL;
    deck->parent = NULL;
    card_t* ncard = deck;
    char suits[4] = {C, S, D, H};
    for (int i = 0; i < 52; i++)
    {
        card_t* card = malloc(sizeof(card_t));

        ncard->s = suits[(int)floor(i/13.f)];
        ncard->n = (i%13)+1;
        ncard->face = false;
        ncard->child = card;

        card->parent = ncard;
        ncard = card;
    }
    ncard->child = NULL;
    tail = deck;
    while (tail->child) { tail = tail->child; }

    deck_shuffle();

    // propagate stacks
    card_t* next_card = tail;
    for (int i = 0; i < 7; i++)
    {

        stacks[i] = next_card;
        stacks[i]->child = NULL;

        next_card = next_card->parent;
        stacks[i]->parent = NULL;

        card_t* stack_card = stacks[i];

        if (i == 0) stack_card->face = true;

        for (int j = 1; j < i+1; j++)
        {
            if (j == i)
                next_card->face = true;

            stack_card->child = next_card;
            next_card->child = stack_card;
            card_t* parent = stack_card;

            stack_card = next_card;
            next_card = next_card->parent;
            stack_card->parent = parent;

        }
        stack_card->child = NULL;
    }
    next_card->child = NULL;
    tail = deck;
    while (tail->child) { tail = tail->child; }
}

static void free_game()
{
    // free stacks
    for (int i = 0; i < 7; i++)
    {
        card_t* card = stacks[i];

        while (card != NULL)
        {
            card_t* next = card->child;
            free(card);
            card = next;
        }
        stacks[i] = NULL;
    }
    free(h_stack); h_stack = NULL;
    free(d_stack); d_stack = NULL;
    free(c_stack); c_stack = NULL;
    free(s_stack); s_stack = NULL;
    card_t* card = deck;
    while (card != NULL)
    {
        card_t* next = card->child;
        free(card);
        card = next;
    }
    card = NULL;
    deck = NULL;
    flip = NULL;
    tail = NULL;
}

int main(int argc, char** argv)
{

    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(900, 720, "E2");
    SetTargetFPS(0);
    SetExitKey(KEY_F12);

    Image i = (Image){(void*)taffer, 160, 160, 1, 7};
    Texture texture = LoadTextureFromImage(i);

    e2_init(texture, GetScreenWidth()/10, GetScreenHeight()/10);

    init_game();

    bool skip_frame = false;
    while (!WindowShouldClose())
    {
        if (h_stack && d_stack && c_stack && s_stack) {
            if (h_stack->n == 13 && d_stack->n == 13 && c_stack->n == 13 && s_stack->n == 13) {
                e2_print("YOU WIN", rand()%e2_get_width(), rand()%e2_get_height(), 0x0b);

                if (g2_button("New Game", 75, 6, 0x70, ALIGN_DEFAULT))
                {
                    free_game();
                    init_game();
                }

                BeginDrawing();
                ClearBackground(BLACK);

                e2_render();

                EndDrawing();

                continue;
            }
        }

        e2_clear(0x0000);

        if (g2_button("New Game", 75, 6, 0x70, ALIGN_DEFAULT))
        {
            free_game();
            init_game();
        }

        if (deck)
        {
            tail = deck;
            while (tail->child) { tail = tail->child; }
        }

        clicked = false;

        if (deck != NULL)
            e2_rich_print(card_back, 5, 3);

        e2_rich_print(card_H, 29+12*0, 3);
        e2_rich_print(card_D, 29+12*1, 3);
        e2_rich_print(card_C, 29+12*2, 3);
        e2_rich_print(card_S, 29+12*3, 3);

        if (h_stack) e2_rich_print(get_card(h_stack), 29+12*0, 3);
        if (d_stack) e2_rich_print(get_card(d_stack), 29+12*1, 3);
        if (c_stack) e2_rich_print(get_card(c_stack), 29+12*2, 3);
        if (s_stack) e2_rich_print(get_card(s_stack), 29+12*3, 3);

        if (click_box(29+12*0, 3, 7, 11) && dragging && mouse_card->s == H && mouse_card->child == NULL)
        {
            if ((h_stack == NULL && mouse_card->n == 1) || (h_stack != NULL && h_stack->n == mouse_card->n-1))
            {
                free(h_stack);
                h_stack = mouse_card;
                mouse_card = NULL;
                dragging = false;
                card_t* end = stacks[drag_from];
                while (end != NULL && end->child != NULL) { end = end->child; }
                if (end != NULL)
                    end->face = true;
            }
        }
        if (click_box(29+12*1, 3, 7, 11) && dragging && mouse_card->s == D && mouse_card->child == NULL)
        {
            if ((d_stack == NULL && mouse_card->n == 1) || (d_stack != NULL && d_stack->n == mouse_card->n-1))
            {
                free(d_stack);
                d_stack = mouse_card;
                mouse_card = NULL;
                dragging = false;
                card_t* end = stacks[drag_from];
                while (end != NULL && end->child != NULL) { end = end->child; }
                if (end != NULL)
                    end->face = true;
            }
        }
        if (click_box(29+12*2, 3, 7, 11) && dragging && mouse_card->s == C && mouse_card->child == NULL)
        {
            if ((c_stack == NULL && mouse_card->n == 1) || (c_stack != NULL && c_stack->n == mouse_card->n-1))
            {
                free(c_stack);
                c_stack = mouse_card;
                mouse_card = NULL;
                dragging = false;
                card_t* end = stacks[drag_from];
                while (end != NULL && end->child != NULL) { end = end->child; }
                if (end != NULL)
                    end->face = true;
            }
        }
        if (click_box(29+12*3, 3, 7, 11) && dragging && mouse_card->s == S && mouse_card->child == NULL)
        {
            if ((s_stack == NULL && mouse_card->n == 1) || (s_stack != NULL && s_stack->n == mouse_card->n-1))
            {
                free(s_stack);
                s_stack = mouse_card;
                mouse_card = NULL;
                dragging = false;
                card_t* end = stacks[drag_from];
                while (end != NULL && end->child != NULL) { end = end->child; }
                if (end != NULL)
                    end->face = true;
            }
        }

        // draw stacks
        for (int i = 0; i < 7; i++)
        {
            card_t* card = stacks[i];
            card_t* top = card;

            int count = 0;
            card_t* pick_card = card;
            bool can_pick = true;
            bool did_pick = false;
            card_t* first_card = NULL;
            int pick_index = -1;
            while (card != NULL)
            {
                if (card->parent && card->parent->face && !card->face && dragging) {
                    e2_rich_print(get_card(card), 3, 30);
                }
                e2_rich_print(card->face?get_card(card):card_back, 5+i*12, 15+count*2);
                if (click_box(5+i*12, 15+count*2, 7, 11) && !mouse_card && card->face)
                {
                    first_card = card;

                    did_pick = true;
                }
                // TODO: check alignment
                count++;
                card = card->child;
            }

            top = first_card;
            while (top!=NULL && top->child != NULL) {
                if (!can_stack(top->child, top))
                {
                    can_pick = false;
                    break;
                }
                top = top->child;
            }

            if (did_pick && can_pick)
            {
                if (first_card->parent == NULL)
                {
                    stacks[i] = NULL;
                } else
                    first_card->parent->child = NULL;
                mouse_card = first_card;
                drag_from =  i;
                dragging = true;
                skip_frame = true;
            }
        }

        // clicked deck
        if (click_box(5, 3, 7, 11) && mouse_card == NULL && deck != NULL)
        {
            flip_deck();
            if (deck == tail)
            {
                deck = NULL;
            }
        }

        // clicked flipped stack
        if (click_box(17, 3, 7, 11) && mouse_card == NULL && flip != NULL)
        {
            mouse_card = flip;

            if (flip->parent != NULL)
            {
                flip->parent->child = flip->child;
                if (flip->child != NULL)
                {
                    flip->child->parent = flip->parent;
                }
            } else
            {
                deck = flip->child;
                if (deck != NULL)
                    deck->parent = NULL; // missed this one fucking line kms
            }

            flip = flip->parent;

            mouse_card->child = NULL;
            mouse_card->parent = NULL;
            mouse_card->face = true;

            dragging = true;
            drag_from = -1;

        }

        // draw flipped stack
        if (flip != NULL)
        {
            e2_rich_print(get_card(flip), 17, 3);
        }

        if (dragging && !skip_frame)
        {
            // draw currently dragged card
            card_t* card = mouse_card;
            int it = 0;
            while (card != NULL)
            {
                e2_rich_print(get_card(card), e2_get_mouse().x-3, e2_get_mouse().y-5+it*2+4);
                it++;
                card = card->child;
            }

            // reset card
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
            {
                if (drag_from == -1)
                {
                    if (flip == NULL)
                    {
                        deck->parent = mouse_card;
                        mouse_card->child = deck;
                        deck = mouse_card;
                    }
                    else
                    {
                        // attempt 1
                        mouse_card->child = flip->child;
                        if(flip->child)
                            flip->child->parent = mouse_card;
                        flip->child = mouse_card;
                        mouse_card->parent = flip;
                    }
                    flip = mouse_card;
                }
                else
                {
                    if (stacks[drag_from] == NULL)
                    {
                        stacks[drag_from] = mouse_card;
                    }
                    else
                    {
                        card_t* end = stacks[drag_from];
                        while (end->child != NULL) { end = end->child; }
                        end->child = mouse_card;
                    }
                }

                mouse_card = NULL;
                dragging = false;
            }

            // drop card
            for (int i = 0; i < 7; i++)
            {
                card_t* ccard = stacks[i];

                if (click_box(5+i*12, 15, 7, 255) && !IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
                {
                    bool dropped = false;

                    if (ccard == NULL)
                    {
                        if (mouse_card->n == 13)
                        {
                            dropped = true;
                            stacks[i] = mouse_card;
                            stacks[i]->parent = NULL;
                        }
                    }
                    else
                    {
                        while (ccard->child != NULL) ccard = ccard->child;
                        if (can_stack(mouse_card, ccard))
                        {
                            dropped = true;
                            mouse_card->parent = ccard;
                            ccard->child = mouse_card;
                        }
                    }

                    if (dropped)
                    {
                        // flip card from stack
                        card_t* end = stacks[drag_from];
                        while (end != NULL && end->child != NULL) { end = end->child; }
                        if (end != NULL)
                            end->face = true;
                        // reset drag settings
                        mouse_card = NULL;
                        dragging = false;
                    }
                }

            }
        }
        if (skip_frame) skip_frame = false;

        BeginDrawing();
        ClearBackground(BLACK);

        e2_render();

        EndDrawing();
    }

    e2_close();

    CloseWindow();

    return 0;

}
