#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <iron/types.h>
#include <iron/mem.h>
#include "game.h"
#include "hash.h"
#include "xxhash.h"

struct _hashstate{
  XXH64_state_t * xxh;
};
  
hashstate * hashstate_new(){
  hashstate hs;
  hs.xxh = XXH64_createState();
  return iron_clone(&hs, sizeof(hs));
}

void hashstate_reset(hashstate * hs){
  XXH64_reset(hs->xxh, 0);
}

void hashstate_update(hashstate * hs, const void * data, size_t len){
  XXH64_update(hs->xxh, data, len);
}

u64 hashstate_digest(hashstate * hs){
  return XXH64_digest(hs->xxh);
}

void hashstate_free(hashstate ** _hs){
  hashstate * hs = *_hs;
  XXH64_freeState(hs->xxh);
  *_hs = NULL;
}
