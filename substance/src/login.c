#include "statelist.h"
#include "e2.h"

void state_login_init()
{
    INIT_STATE(login);
}

void state_login_enter()
{
    
}

void state_login_update()
{
    
}

void state_login_draw()
{
    e2_rich_print("Hello from {#1}state", 1, 2);
}

