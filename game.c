#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <iron/types.h>
#include "game.h"

void game_iteration(controller controller, float dt, game_data * gd){
  physics_update(gd, dt);
  ai_update(gd, dt);
  player_update(gd, controller, dt);
}

/*game_floor game_floor_load(asset_manager am, vec3 pos, vec3 size, const char * name){
  game_floor floor;
  floor.visual = asset_manager_load(am, name, ".png");
  floor.physical = asset_manager_load(am, name, ".height.png");
  floor.pos = pos;
  floor.size = size;
  return floor;
  }*/
