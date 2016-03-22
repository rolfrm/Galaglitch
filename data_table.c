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

table_def * data_table_get_def(){
  static table_def def;
  if(def.cnt == 0){
    column_def columns[] = { COLUMN_DEF(data_table, type, u32),
			     COLUMN_DEF(data_table, data, u64)};
    def.columns = iron_clone(columns, sizeof(columns));
    def.cnt = array_count(columns);
    def.total_size = sizeof(data_table);
  }
  return &def;
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
    void ** array = ((void *) tb) + tb->def->columns[i].offset;
    dealloc(*array);
  }
}

static void table_index_split_check(table_header *  table, table_index tidx, u64 * index, u8 * check){
  *index = tidx.index_data & 0x00FFFFFFFFFFFFFFL;
  *check = (u8)(tidx.index_data >> 56);
  ASSERT(*index > 0);
  ASSERT(table->cnt > *index);
  ASSERT(table->index_check[*index] == *check);
}

table_index _table_add_row(table_header * table){
  static u8 check_hash = 123;
  table_def * def = table->def;
  if(table->unused_index_cnt > 0){
    u64 reuse_index = table->unused_indexes[--(table->unused_index_cnt)];
    ASSERT(reuse_index > 0);
    u8 chk = ++check_hash;
    table->index_check[reuse_index] = chk;
    for(u32 i = 0; i < def->cnt; i++){
      void ** array = ((void *) table) + def->columns[i].offset;
      memset(*array + reuse_index * def->columns[i].size, 0, def->columns[i].size);
    }

    return table_index_new(reuse_index, table->index_check[reuse_index]);
  }
  size_t newcnt = table->cnt + 1;
  for(u32 i = 0; i < def->cnt; i++){
    void ** array = ((void *) table) + def->columns[i].offset;
    *array = realloc(*array, def->columns[i].size * newcnt);
    memset(*array + table->cnt * def->columns[i].size, 0, def->columns[i].size);
  }
  u8 chk = ++check_hash;
  list_push2(table->index_check, table->cnt, chk);
  
  return table_index_new(table->cnt - 1, chk);
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



column_def column_def_new(u32 offset, u32 size, const char * name, void * printer, const char * type_name){
  column_def def;
  def.name = name;
  def.size = size;
  def.offset = offset;
  def.printer = printer;
  def.type_name = type_name;
  return def;
}

#include <stdio.h>

int float_do_print(char * o, int size, float *v){ return snprintf(o, size ,"%f", *v);}
int u32_do_print(char * o, int size, u32 * v){return snprintf(o, size, "%i", *v);}
int u64_do_print(char * o, int size, u64 * v){return snprintf(o, size, "%i", *v);}
int u8_do_print(char * o, int size, u8 * v){return snprintf(o, size, "%i", *v);}
int int_do_print(char * o, int size, int * v){return snprintf(o, size, "%i", *v);}
int table_index_do_print(char * o, int size, table_index * t){
  u64 index = t->index_data & 0x00FFFFFFFFFFFFFFL;
  u8 check = (u8)(t->index_data >> 56);
  return snprintf(o, size ,"%i(%i)", index, check);
}

void _table_print(table_header * table){
  table_def * def = table->def;
  int column_padding[def->cnt + 1];
  for(u32 i = 0; i < def->cnt; i++){
    column_padding[i] = snprintf(NULL, 0, " %s : %s ", def->columns[i].name, def->columns[i].type_name);
    for(u64 j = 0; j < table->cnt; j++){
      void ** array = ((void *)table) + def->columns[i].offset;
      void * ptr = *array + j * def->columns[i].size;
      int s =  def->columns[i].printer(NULL, 0, ptr) + 1;
      column_padding[i] = MAX(s, column_padding[i]);
    }

  }

  column_padding[def->cnt] = 0;
  {
    int index_number_pad = snprintf(NULL, 0, "%i(254)", table->cnt);
    int index_header_pad = snprintf(NULL, 0, "index");
    column_padding[def->cnt] = MAX(index_number_pad, index_header_pad);

  }
  int max_pad = 0;
  for(u32 i = 0; i < (def->cnt + 1); i++)
    max_pad = MAX(max_pad, column_padding[i]);
  
  char buffer[max_pad + 1];
  int s = snprintf(buffer, max_pad, "index");
  printf("| %*sindex |", column_padding[def->cnt] - s, "");
  for(u32 i = 0; i < def->cnt; i++){
    int s = snprintf(NULL, 0, " %s : %s ", def->columns[i].name, def->columns[i].type_name);
    printf("%*s",column_padding[i] - s, "");
    printf(" %s : %s ", def->columns[i].name , def->columns[i].type_name);
    printf(" |");
  }
  printf("\n");
  for(u64 j = 0; j < table->cnt; j++){
    int s = snprintf(buffer,column_padding[def->cnt] + 1,"%i(%i)", j,table->index_check[j]);
    printf("| %*s%s |", column_padding[def->cnt] - s,"", buffer);
    for(u32 i = 0; i < def->cnt; i++){
 
      void ** array = ((void *)table) + def->columns[i].offset;
      void * ptr = *array + j * def->columns[i].size;
      int s =  def->columns[i].printer(NULL, 0, ptr);
      printf("%*s", column_padding[i] - s, "");
      def->columns[i].printer(buffer, column_padding[i], ptr);
      printf("%s", buffer);
      printf(" |");
    }
    printf("\n");
  }
}
