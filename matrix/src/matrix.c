#include "matrix.h"

void matrix_init(int speed)
{
    srand(time(NULL));
    
    _runs = malloc(sizeof(_run_t)*e2_get_width());
    
    for (int i = 0; i < e2_get_width(); i++)
    {
        _runs[i].pos = rand()%e2_get_height();
        _runs[i].len = 20+rand()%30;
        _runs[i].speed = speed + ((rand()%100)/100.f)*20;
        _runs[i].life = 0;
    }
    _time = 0;
    _speed = speed;
}

void matrix_run()
{
    int heck = (e2_get_height()-1);
    
    for (int i = 0; i < e2_get_width(); i++) {
        _run_t* r = &_runs[i];
        r->life += GetFrameTime();
        if (r->life > 1.f/r->speed) {
            for (int j = 1; j < r->len; j++) {
                int a = heck-(((heck-r->pos)+j)%e2_get_height());
                e2_putc(0x00, i, a, 0x00);
            }
            r->pos = (r->pos+1)%e2_get_height();
            
            r->life -= 1.f/r->speed;
        }
        
    }
    
    for (int i = 0; i < e2_get_width(); i++)
    {
        _run_t r = _runs[i];
        int tail = heck-(((heck-r.pos)+r.len)%e2_get_height());
        
        for (int j = 1; j < r.len; j++) {
            int a = heck-(((heck-r.pos)+j)%e2_get_height());
            srand(e2_get_width()*a+i);
            float b = 7.f*((float)j/r.len);
            e2_putc((rand()%256), i, a, 0x70-((int)ceil(b)<<4));
        }
        e2_putc('#', i, r.pos, 0x70);
    }
}
