#include "clock.h"
#include "e2.h"
#include "g1.h"
#include "statelist.h"
#include <raylib.h>

#define alloc(type) malloc(sizeof(type))

static gui_layout_t* _gui;

typedef struct window_t window_t;
typedef void (*window_render_ptr)(window_t* window);

typedef struct widget_t
{
    u16 offset_x;
    u16 offset_y;
    gui_child_t* child;
} widget_t;

struct window_t
{
    char title[16];
    i16 x;
    i16 y;
    u16 w;
    u16 h;
    bool dragging;
    u16 df_x;
    u16 df_y;
    u16 old_x;
    u16 old_y;
    gui_layout_t* panel;
    list_t* widgets;
};
static list_t* _windows;

static window_t* create_window(const char* title, gui_layout_t* panel, u16 x, u16 y, u16 w, u16 h)
{
    LOG("Create window");
    window_t* window = alloc(window_t);
    strcpy(window->title, title);
    window->x = x;
    window->y = y;
    window->w = w;
    window->h = h;
    window->dragging = false;
    window->widgets = list_create(8);
    window->panel = panel;

    for (u16 i = 0; i < panel->children->entries; i++)
    {
        gui_child_t* child = panel->children->data[i];

        LOG("Create widget");
        widget_t* wid = alloc(widget_t);
        wid->offset_x = child->x;
        wid->offset_y = child->y;
        child->x = 0;
        child->y = 0;
        wid->child = child;
        list_append(window->widgets, wid);
    }

    list_push(_windows, window);

    return window;
}

static void update_window(window_t* win)
{
    if (!win)
        return;
    if (win != _windows->data[0])
        return;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && e2_get_mouse().x >= win->x && e2_get_mouse().x < win->x + win->w && e2_get_mouse().y == win->y)
    {
        win->dragging = true;
        win->df_x = e2_get_mouse().x;
        win->df_y = e2_get_mouse().y;
        win->old_x = win->x;
        win->old_y = win->y;
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && win->dragging)
    {
        win->dragging = false;
    }

    if (win->dragging)
    {
        int dx = e2_get_mouse().x - win->df_x;
        int dy = e2_get_mouse().y - win->df_y;
        win->x = win->old_x + dx;
        win->y = win->old_y + dy;
    }
}

static window_t* pick_window(u16 mx, u16 my)
{

    u8 count = 0;
    u8 index = 0;
    bool found = false;

    // find top most clicked window
    for (u8 i = _windows->entries; i > 0; i--)
    {
        window_t* win = _windows->data[i - 1];

        if (mx >= win->x && mx < win->x + win->w && my >= win->y - 1 && my < win->y + win->h)
        {
            count++;
            found = true;

            index = i - 1;
        }
    }

    if (found)
    {
        // sort up
        for (u8 i = index; i > 0; i--)
        {
            window_t* temp = _windows->data[i - 1];
            _windows->data[i - 1] = _windows->data[i];
            _windows->data[i] = temp;
        }

        // return window
        return _windows->data[index];
    }

    return NULL;
}

static void render_window(window_t* win)
{
    if (!win)
        return;

    for (u16 i = 0; i < win->widgets->entries; i++)
    {
        widget_t* widget = win->widgets->data[i];
        gui_child_t* child = widget->child;
        child->x = win->x + widget->offset_x;
        child->y = win->y + widget->offset_y;

        if (win != _windows->data[0])
            child->has_focus = false;
    }
    for (u8 x = 0; x < win->w; x++)
    {
        for (u8 y = 0; y < win->h; y++)
        {
            e2_putc(0xb0, win->x + x + 1, win->y + y + 1, 0x03);
            e2_putc(0x00, win->x + x, win->y + y, 0x40);
        }
    }
    e2ext_box(win->x, win->y, win->w, win->h, 0x40, 0x44);
    e2_print(win->title, win->x + 3, win->y, 0x48);
    e2_putc(0xb4, win->x + 2, win->y, 0x40);
    e2_putc(0xc3, win->x + 3 + strlen(win->title), win->y, 0x40);
    if (e2_get_mouse().x == win->x + win->w - 1 && e2_get_mouse().y == win->y)
    {
        e2_putc('x', win->x + win->w - 1, win->y, 0x84);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            list_delete(_windows, win, true);
        }
    }
    else
    {
        e2_putc('x', win->x + win->w - 1, win->y, 0x48);
    }
}

static void _password_input(char* text, u16 len)
{
    printf("%s\n", text);
}

static gui_child_t* _pass = NULL;
static char* _text;
static char* _orig_text;
static long _sz_text;

static void _scroll_text()
{
    if (_text == _orig_text + _sz_text)
    {
        _text = _orig_text;
        return;
    }
    char* t = _text;
    for (char c = *t; c != '\0'; c = *++t)
    {
        if (c == '\n')
        {
            _text = ++t;
            break;
        }
    }
}

void state_login_init()
{
    INIT_STATE(login);

    clock_init_pool();
    clock_create(125, _scroll_text);

    _windows = list_create(8);
    _gui = gui_create();

    FILE* fi = fopen("420.txt", "rb");
    fseek(fi, 0, SEEK_END);
    _sz_text = ftell(fi);
    _text = malloc(sizeof(char) * (_sz_text + 1));
    memset(_text, 0, sizeof(char) * (_sz_text + 1));

    fseek(fi, 0, SEEK_SET);
    fread(_text, sizeof(char) * _sz_text, 1, fi);
    fclose(fi);

    _orig_text = _text;
}

void state_login_enter()
{
}

void state_login_update()
{
    clock_update_pool(GetFrameTime());
}

static void name_inp(char* text, u16 len)
{
    LOG(text);
}

static bool make_window = false;
static window_t* top_window = NULL;

#define X(x) #x
static u8 d = 7;
static u8 m = 9;
static u16 y = 2021;
void state_login_draw()
{

    if (IsKeyPressed(KEY_O) && IsKeyDown(KEY_LEFT_CONTROL))
    {
        gui_layout_t* date_selector = NULL;
        if (!date_selector)
        {
            date_selector = gui_create();
            gui_add_child(date_selector, gui_calender(4, 4, &d, &m, &y));
        }

        create_window("Calender Test", date_selector, 2, 2, 30, 35);
    }

    if (IsKeyPressed(KEY_S) && IsKeyDown(KEY_LEFT_CONTROL))
    {
        static gui_layout_t* add_subs = NULL;
        if (!add_subs)
        {
            add_subs = gui_create();
            gui_add_child(add_subs, gui_text_input("Name", 4, 4, 16, 15, false, NULL));
            gui_add_child(add_subs, gui_button("Okay", 4, 8, 0, 0x4c, NULL));
            gui_add_child(add_subs, gui_button("Cancel", 15, 8, 0, 0x48, NULL));
        }

        create_window("Add", add_subs, 2, 2, 25, 12);
    }

    if (IsKeyPressed(KEY_H) && IsKeyDown(KEY_LEFT_CONTROL))
    {
        gui_layout_t* test_gui = NULL;
        if (!test_gui)
        {
            test_gui = gui_create();

            static const char* test_txt =
                "{@40}This is a rich\n"
                "text thing.\n"
                "{@4a}woooo{#1}ooo o\n"
                "{#0}{@4c}oo o o {@48}o ooo";

            static const char* rain_txt =
                "{#1}\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\n"
                "\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\n"
                "\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\n"
                "\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\n"
                "\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\n"
                "\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\n"
                "\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\n"
                "\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xdb";
            gui_add_child(test_gui, gui_rich_label(rain_txt, 2, 2));
        }

        create_window("Rich Text", test_gui, 2, 2, 25, 12);
    }

    if (IsKeyPressed(KEY_E) && IsKeyDown(KEY_LEFT_CONTROL))
    {
        static const char* skull_txt =
            "{@48}{#1}  _____ \n"
            " /     \\\n"
            " |() ()| \n"
            " \\  ^  / \n"
            "  ||||| \n"
            "  ||||| \n"
            "  ||||| \n";
        gui_layout_t* skull = gui_create();
        gui_add_child(skull, gui_rich_label(skull_txt, 2, 2));
        gui_add_child(skull, gui_rich_label("{@40}shits bitchin'", 12, 2));
        gui_add_child(skull, gui_button("hella", 20, 6, 0, 0x48, NULL));

        create_window("alert!", skull, 2, 2, 29, 12);
    }

    if (IsKeyPressed(KEY_M) && IsKeyDown(KEY_LEFT_CONTROL))
    {
        gui_layout_t* minesweeper = gui_create();
        gui_add_child(minesweeper, gui_minesweeper(1, 1, 8, 8));

        create_window("Mines", minesweeper, 2, 2, 10, 12);
    }

    if (IsKeyPressed(KEY_Q) && IsKeyDown(KEY_LEFT_CONTROL) && _windows->entries > 0)
    {
        list_remove(_windows, 0, true);
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        window_t* w = pick_window(e2_get_mouse().x, e2_get_mouse().y);
    }

    for (u8 i = _windows->entries; i > 0; i--)
        update_window(_windows->data[i - 1]);

    e2_clear(0x5555);

    e2_rich_print(_text, 0, 0, 0x54);

    for (i8 i = _windows->entries - 1; i >= 0; i--)
    {
        window_t* win = _windows->data[i];

        render_window(win);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && i > 0)
        {
            e2vec2_t mpos = e2_get_mouse();
            bool overlapped = false;
            for (i8 n = i - 1; n >= 0; n--)
            {
                if (mpos.x >= win->x &&
                    mpos.x < win->x + win->w &&
                    mpos.y >= win->y &&
                    mpos.y < win->y + win->h)
                {
                    overlapped = true;
                    break;
                }
            }
            if (overlapped)
            {
                continue;
            }
        }
        gui_update(win->panel);
    }
}
