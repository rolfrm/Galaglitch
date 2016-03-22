// test main
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iron/types.h>
#include <iron/log.h>
#include <iron/test.h>
#include <iron/mem.h>
#include <iron/array.h>
#include <iron/time.h>
#include <iron/utils.h>
#include <iron/linmath.h>

#include <iron/log.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include "data_table.h"
#include "string_table.h"
#include "game.h"

void _error(const char * file, int line, const char * msg, ...){
  char buffer[1000];  
  va_list arglist;
  va_start (arglist, msg);
  vsprintf(buffer,msg,arglist);
  va_end(arglist);
  loge("%s\n", buffer);
  loge("Got error at %s line %i\n", file,line);
  iron_log_stacktrace();
  raise(SIGINT);
  exit(10);
}


u64 TEST_TYPE_1, TEST_TYPE_2;
bool data_table_core_test(){
  TABLE_TYPE(TEST_TYPE_1);
  TABLE_TYPE(TEST_TYPE_2);

  data_table dtt;
  table_header *head = &dtt.header;
  TEST_ASSERT_EQUAL((void *) head, (void*) &dtt);
  data_table * loc = (data_table *) head;
  void * loc2 = &loc->type;
  size_t loc_offset = (size_t)(loc2 - (void *)loc);
  size_t loc_offset2 = (size_t)data_table_get_def()->columns[0].offset; 
  TEST_ASSERT_EQUAL(loc_offset, loc_offset2 );
  logd("Offsets: %i %i\n", loc_offset, loc_offset2);
  
  data_table * dt = table_new(data_table);
  table_index indexes[10];
  for(int i = 0; i < 10; i++){
    indexes[i] = table_add_row(dt);//data_table_insert(dt, TEST_TYPE_2, i * 10 + 50);
    u64 idx = table_raw_index(dt, indexes[i]);
    dt->type[idx] = TEST_TYPE_2;
    dt->data[idx] = i * 10 + 50;
  }

  for(u64 i = 0; i < 10; i++){
    u64 idx = table_raw_index(dt, indexes[i]);
    TEST_ASSERT_EQUAL(dt->data[idx], i * 10 + 50);
    TEST_ASSERT_EQUAL(dt->type[idx], TEST_TYPE_2);
  }
  TABLE_TYPE(TEST_TYPE_1);
  TABLE_TYPE(TEST_TYPE_2);
  for(int i = 0; i < 5; i++){
    table_remove(dt, indexes[i]);
  }
  TEST_ASSERT_EQUAL(dt->header.unused_index_cnt, 5);
  for(int i = 0; i < 5; i++){
    indexes[i] = table_add_row(dt);//, TEST_TYPE_1, i * 200 + 5);
    u64 idx = table_raw_index(dt, indexes[i]);
    dt->data[idx] = i * 200 + 5;
    dt->type[idx] = TEST_TYPE_1;
  }
  
  for(u64 i = 0; i < 5; i++){
    u64 idx = table_raw_index(dt, indexes[i]);
    TEST_ASSERT_EQUAL(dt->data[idx], i * 200 + 5);
    TEST_ASSERT_EQUAL(dt->type[idx], TEST_TYPE_1);
  }
  table_print(dt);
  table_delete(&dt);
  return TEST_SUCCESS;
}

typedef struct{
  table_header header;
  table_index * index0;
  table_index * index1;
  table_index * index2;
}span3_table;

int vec3_do_print(char * o, int size, vec3 *v){
  return snprintf(o, size, "(%f %f %f)", v->x, v->y, v->z);
}

table_def * span3_table_get_def(){
  static table_def def;
  if(def.cnt == 0){
    column_def columns[] = {COLUMN_DEF(span3_table, index0, table_index),
			    COLUMN_DEF(span3_table, index1, table_index),
			    COLUMN_DEF(span3_table, index2, table_index)};
			  
    def.total_size = sizeof(span3_table);
    def.columns = iron_clone(columns, sizeof(columns));
    def.cnt = array_count(columns);;
  }
  return &def;
}

table_def * physics_table_get_def(){
  static table_def def;
  if(def.cnt == 0){
    column_def columns[] = {COLUMN_DEF(physics_table, loc, vec3),
			    COLUMN_DEF(physics_table, size, vec3),
			    COLUMN_DEF(physics_table, vel, vec3),
			    COLUMN_DEF(physics_table, mass, float)};
    def.total_size = sizeof(physics_table);
    def.columns = iron_clone(columns, sizeof(columns));
    def.cnt = array_count(columns);
  }
  return &def;
}

typedef struct{
  table_header header;
  table_index * image_asset;
  table_index * name; 
}sprite_table;

table_def * sprite_table_get_def(){
  static table_def def;
  if(def.cnt == 0){
    column_def columns[] = {COLUMN_DEF(sprite_table, image_asset, table_index),
			    COLUMN_DEF(sprite_table, name, table_index)};
    def.columns = iron_clone(columns, sizeof(columns));
    def.cnt = array_count(columns);
    def.total_size = sizeof(sprite_table);
  }
  return &def;
}

bool span_table_test(){
  u64 TT_PLAYER = table_type_new();
  u64 TT_ENEMY = table_type_new();
  
  data_table * dt = table_new(data_table);
  span3_table * table = table_new(span3_table);
  physics_table * ptable = table_new(physics_table);
  COLUMN_DEF(physics_table, mass, float);
  {
    table_index i1 = table_add_row(dt);
    u64 player_index = table_raw_index(dt, i1);
    dt->type[player_index] = TT_PLAYER;
    dt->data[player_index] = 5;
    table_index player = table_add_row(table);
    table->index0[table_raw_index(table, player)] = i1;
    table_index p1 = table_add_row(ptable);
    table->index1[table_raw_index(table, player)] = p1;
    u64 pidx = table_raw_index(ptable,p1);
    ptable->mass[pidx] = 1.0;
    ptable->vel[pidx] = vec3_new(1,2,3);
  }

  {
    table_index i2 = table_add_row(dt);
    u64 enemy_index = table_raw_index(dt, i2);
    dt->type[enemy_index] = TT_ENEMY;
    dt->data[enemy_index] = 4;
    table_index enemy = table_add_row(table);
    table_index p2 = table_add_row(ptable);
    table->index0[table_raw_index(table, enemy)] = i2;
    table->index1[table_raw_index(table, enemy)] = p2;
    ptable->mass[table_raw_index(ptable, p2)] = 2.0;
    ptable->vel[table_raw_index(ptable, p2)] = vec3_new(0,0.2,0.1);
  }
  table_print(table);
  table_print(ptable);
  table_print(dt);
  for(size_t i = 1; i < table->header.cnt; i++){
    
    u64 idx = table_raw_index(dt, table->index0[i]);
    u64 idx2 = table_raw_index(ptable, table->index1[i]);
    logd("Idx: %i %i %i %i %f\n", i, dt->type[idx], dt->data[idx], idx2, ptable->mass[idx2]);
    TEST_ASSERT(idx2 == 0 || ptable->mass[idx2] > 0);
  }
  for(int j = 0; j < 5; j++){
    table_print(ptable);
    for(size_t i = 1; i < ptable->header.cnt; i++){
      ptable->loc[i] = vec3_add(ptable->loc[i], ptable->vel[i]);
    }
  }
  printf("Span3 table:\n");
  table_print(table);
  table_print(ptable);
  table_print(dt);
  table_delete(&ptable);
  table_delete(&dt);
  table_delete(&table);
  
  return TEST_SUCCESS;
}

bool string_table_test(){
  string_table * table = table_new(string_table);
  string_table_insert(table, "Hello?");
  string_table_insert(table, "Hello?");
  string_table_insert(table, "Helloma?");
  string_table_insert(table, "Red?");
  string_table_insert(table, "Blue?");
  string_table_insert(table, "Red?");
  string_table_insert(table, "Green?");
  string_table_insert(table, "Green!");
  string_table_insert(table, "Purple");
  string_table_insert(table, "purple");
  string_table_insert(table, "purple-grayish");
  string_table_insert(table, "lavender");
  string_table_insert(table, "alice-blue");
  table_index idx = string_table_get(table, "Blue?");
  ASSERT(idx.index_data != 0);
  table_print(table);
  return TEST_SUCCESS;
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

/*bool test_gl_image_table()
{
  gl_image_table * gl_tab = table_new(gl_image_table);
  string_table * s_tab = table_new(string_table);
  table_index _idx = table_add_row(gl_tab);
  u64 idx = table_raw_index(gl_tab, _idx);
  
  }*/

bool game_content_test(){
  game_content * content = init_game_content();
  TEST_ASSERT(content != NULL);

  table_index idx = table_add_row(content->entities);
  content->entities->name[idx.raw] = string_table_insert(content->strings, "Player");
  table_print(content->entities);
  table_print(content->strings);
  return TEST_SUCCESS;
}

int main(){
  TEST(data_table_core_test);
  TEST(span_table_test);
  TEST(string_table_test);
  TEST(game_content_test);
  //TEST(test_line_segment);
  //TEST(test_main_loop);
  //TEST(test_graphics);
  //TEST(test_distance_field);
  return 0;
}
