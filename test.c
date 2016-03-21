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
#include "game.h"

#include <iron/log.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include "data_table.h"

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
  size_t loc_offset2 = (size_t)data_table_get_def()->offset[0]; 
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
  table_delete(&dt);
  return TEST_SUCCESS;
}
typedef struct{
  size_t cnt;
  size_t column_cnt;
  table_index ** indexes;

}span_table;

table_index span_table_insert(span_table * span, const table_index * indexes);
span_table * span_table_new(int column_cnt){
  span_table * table = alloc0(sizeof(span_table) + column_cnt * sizeof(void *));
  table->indexes = ((void *)table) + offsetof(span_table, indexes) + sizeof(void *);
  table->column_cnt = column_cnt;
  table_index zero_indexes[table->column_cnt];
  memset(zero_indexes, 0, sizeof(zero_indexes));
  span_table_insert(table,zero_indexes);
  return table;
}

table_index span_table_insert(span_table * span, const table_index * indexes){
  span->cnt += 1; 
  for(size_t i = 0; i < span->column_cnt; i++){
    span->indexes[i] = realloc(span->indexes[i], sizeof(table_index) * span->cnt);
    span->indexes[span->cnt - 1][i] = indexes[i];
  }
  return table_index_new(span->cnt - 1, 0);
}

void span_table_delete(span_table ** table_loc){
  span_table * table = *table_loc;
  *table_loc = NULL;
  for(size_t i = 0; i < table->column_cnt; i++)
    dealloc(table->indexes[i]);
  dealloc(table);
}

bool span_table_test(){
  u64 TT_PLAYER = table_type_new();
  u64 TT_ENEMY = table_type_new();
  
  data_table * dt = table_new(data_table);
  span_table * table = span_table_new(3);
  table_index indexes[3] = {table_index_default, table_index_default, table_index_default};
  indexes[0] = table_add_row(dt);
  u64 player_index = table_raw_index(dt, indexes[0]);
  dt->type[player_index] = TT_PLAYER;
  dt->data[player_index] = 5;
  
  table_index player = span_table_insert(table, indexes);
  indexes[0] = table_add_row(dt);
  u64 enemy_index = table_raw_index(dt, indexes[0]);
  dt->type[enemy_index] = TT_ENEMY;
  dt->data[enemy_index] = 4;
  table_index enemy = span_table_insert(table, indexes);
  logd("Player: %i, Enemy: %i\n", player.index_data, enemy.index_data);
  for(size_t i = 0; i < table->cnt; i++){
    
    u64 idx = table_raw_index(dt, table->indexes[i][0]);
    logd("Idx: %i %i %i\n", i, dt->type[idx], dt->data[idx]);
  }
  table_delete(&dt);
  span_table_delete(&table);
  
  return TEST_SUCCESS;
}

int main(){
  TEST(data_table_core_test);
  TEST(span_table_test);
  //TEST(test_line_segment);
  //TEST(test_main_loop);
  //TEST(test_graphics);
  //TEST(test_distance_field);
  return 0;
}
