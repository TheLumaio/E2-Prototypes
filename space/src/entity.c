#include "entity.h"

void ent_init(world_t* world) {
    _world = world;
}

typedef struct ent_fire_t {
    u16 burn_intensity;
} ent_fire_t;
ent_base_t* ent_fire(chunk_t* chunk, u16 x, u16 y, u16 burn_intensity)
{
    ent_base_t* base = malloc(sizeof(ent_base_t));
    base->ptr = ent_fire_update;
    base->x = x;
    base->y = y;
    base->chunk = chunk;
    base->color = 0x08;
    base->tile = 'f';

    ent_fire_t* ent = malloc(sizeof(ent_fire_t));
    ent->burn_intensity = burn_intensity;

    base->ent = ent;

    return base;
}

void ent_fire_update(ent_base_t* base)
{
    ent_fire_t* ent = base->ent;
    // remove entity when it burns out
    // add coal
    if (ent->burn_intensity == 0)
    {
        u8 tile = '*';
        u8 color = 0x01;

        base->chunk->ents[base->y*WORLD_CHUNK_SIZE+base->x] = NULL;
        base->chunk->data[base->y*WORLD_CHUNK_SIZE+base->x] = (u16)((tile)|(color<<8));
            
        free(ent);
        free(base);
        return;
    }

    // tick down intensity
    ent->burn_intensity--;

    // don't spread of fire has burned out
    if (ent->burn_intensity < 1) return;

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
            chunk_add_ent(n->chunk, ent_fire(n->chunk, n->x, n->y, ent->burn_intensity+3));
        }
        else if (n->tile == ',' || n->tile == '.')
        {
            chunk_add_ent(n->chunk, ent_fire(n->chunk, n->x, n->y, ent->burn_intensity-1));
        }
    }
    
}

