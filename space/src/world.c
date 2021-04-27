#include "world.h"
#include "e2util.h"
#include "entity.h"

world_t* world_load(const char* filename)
{
    if (_world != NULL) {
        LOG("World already loaded.");
        return _world;
    }

    FILE* f = fopen(filename, "rb");
    if (!f)
    {
        LOG("Failed to open map file");
        return NULL;
    }

    _world = malloc(sizeof(world_t));
    _world->chunks = list_create(8);
    _world->to_update = list_create(8);

    while (!feof(f))
    {
        chunk_t* chunk = malloc(sizeof(chunk_t));
        fread(chunk, sizeof(chunk_t)-sizeof(chunk->ents), 1, f);
        memset(chunk->ents, 0, sizeof(chunk->ents));
        list_append(_world->chunks, chunk);
        LOG("Chunk loaded at %d:%d", chunk->x, chunk->y);
    }

    fclose(f);

    LOG("World loaded %d chunks", _world->chunks->entries);

    return _world;
}

world_t* world_create(const char* filename)
{
    _world = malloc(sizeof(world_t));
    _world->filename = malloc(strlen(filename)+1);
    strcpy(_world->filename, filename);

    _world->chunks = list_create(8*8);

    return _world;

}

chunk_t* world_get_chunk(i32 x, i32 y)
{
    if (!_world) return NULL;
    for (i32 k = 0; k < _world->chunks->entries; k++) {
        chunk_t* tmp = _world->chunks->data[k];
        if (!tmp) continue;
        if (tmp->x == x && tmp->y == y){
            return tmp;
        }
    }
    return NULL;
}

void world_save(const char* filename)
{
    FILE* f = fopen(filename, "wb+");
    if (!f)
    {
//        printf("failed to open file \"%s\"\n", filename);
        LOG("Failed to open file %s", filename);
        return;
    }

    for (i32 i = 0; i < _world->chunks->entries; i++)
    {
        fwrite(_world->chunks->data[i], sizeof(chunk_t), 1, f);
    }

    fclose(f);
}

void world_add_chunk(chunk_t* chunk)
{
    list_append(_world->chunks, chunk);
}

list_t* world_get_neighbors(chunk_t* chunk, u16 x, u16 y)
{
    e2vec2_t neighbors[8] = {
        {x-1, y-1},
        {x-0, y-1},
        {x+1, y-1},
        {x+1, y-0},
        {x+1, y+1},
        {x-0, y+1},
        {x-1, y+1},
        {x-1, y-0},
    };

    list_t* list = list_create(8);

    for (i32 i = 0; i < 8; i++)
    {
        e2vec2_t current_pos = neighbors[i];
        chunk_t* node_chunk = chunk;
        if (current_pos.x < 0)
        {
            current_pos.x += WORLD_CHUNK_SIZE;
            node_chunk = world_get_chunk(node_chunk->x-1, node_chunk->y);
        }
        else if (current_pos.x >= WORLD_CHUNK_SIZE)
        {
            current_pos.x -= WORLD_CHUNK_SIZE;
            node_chunk = world_get_chunk(node_chunk->x+1, node_chunk->y);
        }

        if (!node_chunk) continue;  
        
        if (current_pos.y < 0)
        {
            current_pos.y += WORLD_CHUNK_SIZE;
            node_chunk = world_get_chunk(node_chunk->x, node_chunk->y-1);
        }
        else if (current_pos.y >= WORLD_CHUNK_SIZE)
        {
            current_pos.y -= WORLD_CHUNK_SIZE;
            node_chunk = world_get_chunk(node_chunk->x, node_chunk->y+1);
        }
        
        if (node_chunk != NULL)
        {
            world_neighbor_t* n = malloc(sizeof(world_neighbor_t));
            n->chunk = node_chunk;
            n->x = current_pos.x;
            n->y = current_pos.y;
            n->tile = node_chunk->data[n->y*WORLD_CHUNK_SIZE+n->x];
            list_append(list, n);
        }
    }

    return list;
}

world_t* world_get()
{
    return _world;
}

void chunk_update(i32 chunk_x, i32 chunk_y, u16* data)
{

}

i16 chunk_get_tile(chunk_t* chunk, i32 x, i32 y)
{
    return chunk->data[y*WORLD_CHUNK_SIZE+x]&0x00ff;
}

void chunk_add_ent(chunk_t* chunk, ent_base_t* ent)
{
//    list_append(chunk->ents, ent);
    chunk->ents[ent->y*WORLD_CHUNK_SIZE+ent->x] = ent;
    if (!list_has(_world->to_update, chunk))
        list_append(_world->to_update, chunk);

}


