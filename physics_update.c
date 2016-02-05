#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <iron/types.h>
#include "game.h"


void physics_update( game_data * gd, float dt){
  game_entities * entities = &gd->entities;
  for(int i = 0; i < entities->cnt; i++){
    entities->x[i] += entities->dx[i] * dt;
    entities->y[i] += entities->dy[i] * dt;
  }
}
