#include "cutscene.h"

#define INC(a) a = *(script++)

static u8 _scene_cursor_x = 0;
static u8 _scene_cursor_y = 0;
static u16 _scene_speed = 0;
static u8 _scene_effect = EFFECT_NONE;

void scene_parse_script(const char* script)
{
    char* token = malloc(sizeof(char) * 256);
    memset(token, '\0', sizeof(char) * 256);

    char c = *(script);

    while (c != 0)
    {
        LOG("%c", c);
        if (c == '{')
        {
            while (c != '}')
            {
                if (c == '#')
                {
                    token[strlen(token)] = c;
                    INC(c);
                    break;
                }
                INC(c);
            }
        }

        INC(c);
    }
}

void scene_step()
{

}

