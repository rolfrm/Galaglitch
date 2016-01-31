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

