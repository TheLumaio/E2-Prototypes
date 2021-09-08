#ifndef CHUNK_HANDLER
#define CHUNK_HANDLER

#include <stdio.h>
#include <stdint.h>

#include "e2.h"
#include "list.h"

// forward def to avoid recursive includes
typedef struct ent_base_t ent_base_t;

#define WORLD_CHUNK_SIZE 16

#define TILE(ch, co) (ch<<8|co)

typedef enum tile_type_e {
    TILE_INVALID = TILE(0x00, 0x00),
    TILE_GRASS1 = TILE(',', 0x0d),
    TILE_GRASS2 = TILE('.', 0x0e),
    TILE_WATER = TILE('~', 0x0d),
    TILE_TREE1,
    TILE_TREE2,
    TILE_WALL,
    TILE_SAND1,
    TILE_SAND2,
    TILE_FLOOR
} tile_type_e;

typedef struct chunk_t {
    uint16_t data[WORLD_CHUNK_SIZE*WORLD_CHUNK_SIZE];
    i32 x;
    i32 y;
    ent_base_t* ents[WORLD_CHUNK_SIZE*WORLD_CHUNK_SIZE];
    // TODO: entities
} chunk_t;

typedef struct world_t {
    char* filename;
    list_t* chunks;
    list_t* to_update; // list of chunks that need an update
} world_t;

typedef struct world_neighbor_t {
    chunk_t* chunk;
    u16 x;
    u16 y;
    u8 tile;
} world_neighbor_t;

static world_t* _world = NULL;

world_t* world_load(const char* filename);
world_t* world_create(const char* filename);
chunk_t* world_get_chunk(i32 x, i32 y);
void world_save(const char* filename);
void world_add_chunk(chunk_t* chunk);
list_t* world_get_neighbors(chunk_t* chunk, u16 x, u16 y);

world_t* world_get();

void chunk_update(i32 x, i32 y, u16* data);
i16 chunk_get_tile(chunk_t* chunk, i32 x, i32 y);
void chunk_add_ent(chunk_t* chunk, ent_base_t* ent);


#endif
