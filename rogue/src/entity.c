#include "entity.h"
#include "world.h"

void ent_init(world_t* world) {
    _world = world;
}

void ent_destroy(ent_base_t* base)
{
    base->chunk->ents[base->y*WORLD_CHUNK_SIZE+base->x] = NULL;

    free(base->ent);
    free(base);
}

ent_base_t* ent_fire(chunk_t* chunk, u16 x, u16 y, u16 lifetime_ticks)
{
    ent_base_t* base = malloc(sizeof(ent_base_t));
    base->ptr = ent_fire_update;
    base->x = x;
    base->y = y;
    base->chunk = chunk;
    base->color = 0x08;
    base->tile = 'f';

    ent_data_t* ent = malloc(sizeof(ent_data_t));
    ent->lifetime_ticks = lifetime_ticks;

    base->ent = ent;

    return base;
}

void ent_fire_update(ent_base_t* base)
{
    ent_data_t* ent = base->ent;
    // remove entity when it burns out
    // add coal
    if (ent->lifetime_ticks == 0)
    {
        u8 tile = '*';
        u8 color = 0x01;

        base->chunk->data[base->y*WORLD_CHUNK_SIZE+base->x] = (u16)((tile)|(color<<8));

        ent_destroy(base);

        return;
    }

    // tick down intensity
    ent->lifetime_ticks--;

    // don't spread of fire has burned out
    if (ent->lifetime_ticks < 1) return;

    // spread
    list_t*  neighbors = world_get_neighbors(base->chunk, base->x, base->y);
    for (int i = 0; i < 8; i++)
    {
        int r = rand()%10;
        if (r>2) continue;
        world_neighbor_t* n = neighbors->data[i];
        if (n->chunk->ents[n->y*WORLD_CHUNK_SIZE+n->x] != NULL) continue;
        if (n->tile == 0x05 || n->tile == 0x06)
        {
            chunk_add_ent(n->chunk, ent_fire(n->chunk, n->x, n->y, ent->lifetime_ticks+3));
        }
        else if (n->tile == ',' || n->tile == '.')
        {
            chunk_add_ent(n->chunk, ent_fire(n->chunk, n->x, n->y, ent->lifetime_ticks-1));
        }
    }

}

typedef struct ent_lightning_t {
    u16 volts;
} ent_lightning_t;
ent_base_t* ent_lightning(chunk_t* chunk, u16 x, u16 y, u16 volts)
{
    ent_base_t* base = malloc(sizeof(ent_base_t));
    base->ptr = ent_lightning_update;
    base->x = x;
    base->y = y;
    base->chunk = chunk;
    base->color = 0x0d;
    base->tile = '/';

    ent_data_t* ent = malloc(sizeof(ent_data_t));
    ent->volts = volts;
    ent->lifetime_ticks = 6;

    base->ent = ent;

    return base;
}

void ent_lightning_update(ent_base_t* base)
{
    ent_data_t* ent = base->ent;

    /*
    lightning sticks around for a few ticks.
    create sparks around pooint of contact.
    after lightning disappears, create fire ent.
    */

    ent->lifetime_ticks--;

    if (ent->lifetime_ticks == 0)
    {
        ent_destroy(base);

        chunk_add_ent(base->chunk, ent_fire(base->chunk, base->x, base->y, 2));

        return;
    }

    // randomly create sparks
    if (rand()%2 == 0)
    {
        list_t* neighbors = world_get_neighbors(base->chunk, base->x, base->y);
        world_neighbor_t* n = neighbors->data[rand()%8];
        if (n->chunk->ents[n->y*WORLD_CHUNK_SIZE+n->x] == NULL)
        {
            chunk_add_ent(n->chunk, ent_spark(n->chunk, n->x, n->y, ent->lifetime_ticks));
        }
    }

    base->tile = (rand()%2) == 0 ? '/' : '\\';
    base->color = (rand()%2) == 0 ? 0x0d : 0x07;

}

ent_base_t* ent_spark(chunk_t* chunk, u16 x, u16 y, u16 prop)
{
    ent_base_t* base = malloc(sizeof(ent_base_t));
    base->ptr = ent_spark_update;
    base->x = x;
    base->y = y;
    base->chunk = chunk;
    base->color = 0x0d;
    base->tile = '*';

    ent_data_t* ent = malloc(sizeof(ent_data_t));
    ent->volts = 420;
    ent->propagation = prop;
    ent->lifetime_ticks = 2;

    base->ent = ent;

    return base;
}

void ent_spark_update(ent_base_t* base)
{
    ent_data_t* ent = base->ent;

    ent->lifetime_ticks--;
    if (ent->lifetime_ticks == 0)
    {
        ent_destroy(base);
        return;
    }

    list_t* neighbors = world_get_neighbors(base->chunk, base->x, base->y);
    for (u8 i = 0; i < 8; i++)
    {
        world_neighbor_t* n = neighbors->data[i];
        if (n->tile == '~' && ent->propagation > 0 && rand()%3 == 0)
        {
            chunk_add_ent(n->chunk, ent_spark(n->chunk, n->x, n->y, ent->propagation - 1));
        }
    }
    base->color = (rand()%2) == 0 ? 0x0d : 0x07;
}
