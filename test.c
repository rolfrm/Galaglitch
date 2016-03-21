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
bool data_table_test(){
  TABLE_TYPE(TEST_TYPE_1);
  TABLE_TYPE(TEST_TYPE_2);
    
  data_table * dt = data_table_new();
  table_index indexes[10];
  for(int i = 0; i < 10; i++){
    indexes[i] = data_table_insert_item(dt, TEST_TYPE_2, i * 10 + 50);  
  }

  for(u64 i = 0; i < 10; i++){
    u64 data;
    u32 type;
    data_table_data(dt, indexes[i], &type, &data);
    TEST_ASSERT_EQUAL(data, i * 10 + 50);
    TEST_ASSERT_EQUAL(type, TEST_TYPE_2);
  }
  TABLE_TYPE(TEST_TYPE_1);
  TABLE_TYPE(TEST_TYPE_2);
  for(int i = 0; i < 5; i++){
    data_table_remove_item(dt, indexes[i]);
  }
  TEST_ASSERT_EQUAL(dt->unused_index_cnt, 5);
  for(int i = 0; i < 5; i++){
    indexes[i] = data_table_insert_item(dt, TEST_TYPE_1, i * 200 + 5);
  }
  
  for(u64 i = 0; i < 5; i++){
    u64 data;
    u32 type;
    data_table_data(dt, indexes[i], &type, &data);
    TEST_ASSERT_EQUAL(data, i * 200 + 5);
    TEST_ASSERT_EQUAL(type, TEST_TYPE_1);
  }
  data_table_delete(&dt);
  return TEST_SUCCESS;
}


int main(){
  TEST(data_table_test);
  //TEST(test_line_segment);
  //TEST(test_main_loop);
  //TEST(test_graphics);
  //TEST(test_distance_field);
  return 0;
}
