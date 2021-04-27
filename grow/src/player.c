#include "player.h"

float player_get_dosh()
{
    return _dosh;
}

void player_pay()
{
    // apply bonuses or w/e
    _dosh += 1;
}
