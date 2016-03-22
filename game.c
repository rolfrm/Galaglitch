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

table_def * surface_table_get_def(){
  static table_def def;
  if(def.cnt == 0){
    column_def columns[] = {COLUMN_DEF(surface_table, name, table_index),
			     COLUMN_DEF(surface_table, loc, vec3),
			     COLUMN_DEF(surface_table, size, vec3),
			     COLUMN_DEF(surface_table, texture, table_index),
			     COLUMN_DEF(surface_table, height_map, table_index)};
    def.columns = iron_clone(columns, sizeof(columns));
    def.total_size = sizeof(surface_table);
    def.cnt = array_count(columns);
  }
  return &def;
}

table_def * gl_tex_table_get_def(){
  static table_def def;
  if(def.cnt == 0){
    column_def columns[] =
      {COLUMN_DEF(gl_tex_table, name, table_index),
       COLUMN_DEF(gl_tex_table, height, int),
       COLUMN_DEF(gl_tex_table, width, int),
       COLUMN_DEF(gl_tex_table, format, u8),
       COLUMN_DEF(gl_tex_table, gl_ref, u32)
      };
    def.columns = iron_clone(columns, sizeof(columns));
    def.cnt = array_count(columns);
    def.total_size = sizeof(gl_tex_table);
  }
  return &def;
}


game_content * init_game_content(){
  game_content * content = alloc0(sizeof(game_content));
  content->strings = table_new(string_table);
  content->entities = table_new(entity_table);
  content->physics = table_new(physics_table);
  content->data = table_new(data_table);
  content->surfaces = table_new(surface_table);
  content->textures = table_new(gl_tex_table);
  return content;
}
