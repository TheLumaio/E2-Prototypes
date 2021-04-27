#ifndef ENTITY_H
#define ENTITY_H

#include "e2.h"
#include "world.h"

// basic polymorphism stuff
typedef struct ent_base_t ent_base_t;

typedef void (*ent_update_ptr)(ent_base_t*);

struct ent_base_t {
    ent_update_ptr ptr;
    u16 x,y;
    chunk_t* chunk;
    void* ent;
    u8 color;
    u8 tile;

};

static world_t *_world;

void ent_init(world_t* world);

// entities below
ent_base_t* ent_fire(chunk_t* chunk, u16 x, u16 y, u16 burn_intensity);
static void ent_fire_update(ent_base_t* base);

#endif

