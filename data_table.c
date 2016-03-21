#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <iron/types.h>
#include <iron/array.h>
#include <iron/log.h>
#include <iron/mem.h>
#include "data_table.h"

static table_index table_index_new(u64 index, u8 _check){
  u64 check = _check;
  table_index out;
  out.index_data = index | (check << 58);
  return out;
}

data_table * data_table_new(){
  data_table * table =alloc0(sizeof(data_table));
  // add the 0 index.
  data_table_insert_item(table, 0, 0);
  return table;
}

void data_table_delete(data_table ** _tb){
  data_table * tb = *_tb;
  *_tb = NULL;
  dealloc(tb->index_check);
  dealloc(tb->type);
  dealloc(tb->data);
  dealloc(tb->unused_indexes);
}

static void table_index_split_check(data_table *  table, table_index tidx, u64 * index, u8 * check){
  *index = tidx.index_data & 0x00FFFFFFFFFFFFFFL;
  *check = (u8)(tidx.index_data >> 58);
  ASSERT(table->cnt > *index);
  ASSERT(table->index_check[*index] == *check);
}

table_index data_table_insert_item(data_table * table, u32 type, u64 data){
  if(table->unused_index_cnt > 0){
    u64 reuse_index = --(table->unused_index_cnt);
    table->index_check[reuse_index]++;
    table->type[reuse_index] = type;
    table->data[reuse_index] = data;
    return table_index_new(reuse_index, table->index_check[reuse_index]);
  }
    
  list_push(table->type, table->cnt, type);
  list_push(table->index_check, table->cnt, 0);
  list_push2(table->data, table->cnt, data);
  return table_index_new(table->cnt - 1, 0);
}

void data_table_remove_item(data_table * table, table_index t_index){
  u64 index;
  u8 check;
  table_index_split_check(table, t_index, &index, &check);
  table->index_check[index] += 1;
  table->type[index] = 0;
  table->data[index] = 0;
  list_push2(table->unused_indexes, table->unused_index_cnt, index);
}

void data_table_data(data_table * table, table_index t_index, u32 * out_type, u64 * out_data){
  u64 index;
  u8 check;
  table_index_split_check(table, t_index, &index, &check);
  if(out_type != NULL)
    *out_type = table->type[index];
  if(out_data != NULL)
    *out_data = table->data[index];
}

static u32 table_type_cnt = 0;
u32 table_type_new(){
  return ++table_type_cnt;
}

#define TABLE_TYPE(X) (X ? X : (X = table_type_new()));



