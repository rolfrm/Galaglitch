#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <iron/types.h>
#include <iron/array.h>
#include <iron/log.h>
#include <iron/mem.h>
#include <iron/utils.h>
#include <string.h>
#include <stddef.h>
#include "data_table.h"



static u32 data_table_offsets[] = {offsetof(data_table, type), offsetof(data_table, data)};
static u32 data_table_sizes[] = {sizeof(u32), sizeof(u64)};

static table_def data_table_def_ins;
table_def * data_table_get_def(){
  static column_def data_table_columns[2];
  if(data_table_def_ins.cnt == 0){
    data_table_columns[0] = COLUMN_DEF(data_table, type, u32);
    data_table_columns[1] = COLUMN_DEF(data_table, data, u64);
    table_def tdef;
    tdef.size = data_table_sizes;
    tdef.offset = data_table_offsets;
    tdef.columns = data_table_columns;
    tdef.cnt = array_count(data_table_sizes);
    tdef.total_size = sizeof(data_table);
    data_table_def_ins = tdef;
    
  }
  return &data_table_def_ins;
}

table_index table_index_default;

table_index table_index_new(u64 index, u8 _check){
  u64 check = _check;
  table_index out;
  out.index_data = index | (check << 56);
  return out;
}

table_header * _table_new(table_def * def){
  table_header * table = alloc0(def->total_size);
  table->def = def;
  // add the 0 index.
  _table_add_row(table);
  return table;
}

void _table_delete(table_header ** _tb){
  table_header * tb = *_tb;
  *_tb = NULL;
  dealloc(tb->index_check);
  dealloc(tb->unused_indexes);
  for(u32 i = 0; i < tb->def->cnt; i++){
    void ** array = ((void *) tb) + tb->def->offset[i];
    dealloc(*array);
  }
}

static void table_index_split_check(table_header *  table, table_index tidx, u64 * index, u8 * check){
  *index = tidx.index_data & 0x00FFFFFFFFFFFFFFL;
  *check = (u8)(tidx.index_data >> 56);
  ASSERT(table->cnt > *index);
  ASSERT(table->index_check[*index] == *check);
}

table_index _table_add_row(table_header * table){
  if(table->unused_index_cnt > 0){
    u64 reuse_index = --(table->unused_index_cnt);
    table->index_check[reuse_index]++;
    for(u32 i = 0; i < table->def->cnt; i++){
      void ** array = ((void *) table) + table->def->offset[i];
      memset(*array + reuse_index * table->def->size[i], 0, table->def->size[i]);
    }
    return table_index_new(reuse_index, table->index_check[reuse_index]);
  }
  size_t newcnt = table->cnt + 1;
  for(u32 i = 0; i < table->def->cnt; i++){
    void ** array = ((void *) table) + table->def->offset[i];
    *array = realloc(*array, table->def->size[i] * newcnt);
    memset(*array + table->cnt * table->def->size[i], 0, table->def->size[i]);
  }
  list_push2(table->index_check, table->cnt, 0);
  
  return table_index_new(table->cnt - 1, 0);
}

void _table_remove(table_header * table, table_index t_index){
  u64 index;
  u8 check;
  table_index_split_check(table, t_index, &index, &check);
  table->index_check[index] += 1;
  list_push2(table->unused_indexes, table->unused_index_cnt, index);
}

u64 _table_raw_index(table_header * table, table_index t_index){
  u64 index;
  u8 check;
  table_index_split_check(table, t_index, &index, &check);
  return index;
}

static u32 table_type_cnt = 0;
u32 table_type_new(){
  return ++table_type_cnt;
}

#define TABLE_TYPE(X) (X ? X : (X = table_type_new()));



column_def column_def_new(u32 offset, u32 size, const char * name, void * printer){
  column_def def;
  def.name = name;
  def.size = size;
  def.offset = offset;
  def.printer = printer;
  return def;
}

#include <stdio.h>

int float_do_print(char * o, int size, float *v){ return snprintf(o, size ,"%f", *v);}
int u32_do_print(char * o, int size, u32 * v){return snprintf(o, size, "%i", *v);}
int u64_do_print(char * o, int size, u64 * v){return snprintf(o, size, "%i", *v);}
int table_index_do_print(char * o, int size, table_index * t){
  u64 index = t->index_data & 0x00FFFFFFFFFFFFFFL;
  u8 check = (u8)(t->index_data >> 56);
  return snprintf(o, size ,"%i(%i)", index, check);
}

void _table_print(table_header * table){
  int pre_padding = 0;
  table_def * def = table->def;
  for(u32 i = 0; i < def->cnt; i++){
    int s = snprintf(NULL, 0, "%s", def->columns[i].name);
    pre_padding = MAX(s, pre_padding);
    for(u64 j = 0; j < table->cnt; j++){
      void ** array = ((void *)table) + def->columns[i].offset;
      void * ptr = *array + j * def->columns[i].size;
      int s =  def->columns[i].printer(NULL, 0, ptr);
      pre_padding = MAX(s, pre_padding);
    }
  }
  pre_padding += 1;
      
  char buffer[pre_padding];

  for(u32 i = 0; i < def->cnt; i++){
    if(i > 0)
      printf(" |");
    int s = snprintf(NULL, 0, "%s", def->columns[i].name);
    printf("%*s", pre_padding - s, "");
    printf("%s", def->columns[i].name);
  }
  printf("\n");
  for(u64 j = 0; j < table->cnt; j++){
    for(u32 i = 0; i < def->cnt; i++){
      if(i > 0)
	printf(" |");
      void ** array = ((void *)table) + def->columns[i].offset;
      void * ptr = *array + j * def->columns[i].size;
      int s =  def->columns[i].printer(NULL, 0, ptr);
      printf("%*s", pre_padding - s, "");
      def->columns[i].printer(buffer, pre_padding, ptr);
      printf("%s", buffer);

    }
    printf("\n");
  }
}
