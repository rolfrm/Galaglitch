#include <stdbool.h>
#include <stdint.h>
#include <iron/types.h>
#include "data_table.h"
#include "uthash.h"
#include "string_table.h"
#include<iron/linmath.h>
#include "game.h"

#include <iron/mem.h>
#include <iron/utils.h>
#include <stdio.h>

table_def * entity_table_get_def(){
  static table_def def;
  if(def.cnt == 0){
    column_def columns[] = {COLUMN_DEF(entity_table, name, table_index),
			    COLUMN_DEF(entity_table, data1, table_index),
			    COLUMN_DEF(entity_table, data2, table_index),
			    COLUMN_DEF(entity_table, data3, table_index)};
			  
    def.total_size = sizeof(entity_table);
    def.columns = iron_clone(columns, sizeof(columns));
    def.cnt = array_count(columns);
  }
  return &def;
}

game_content * init_game_content(){
  game_content * content = alloc0(sizeof(game_content));
  content->strings = table_new(string_table);
  content->entities = table_new(entity_table);
  content->physics = table_new(physics_table);
  return content;
}
