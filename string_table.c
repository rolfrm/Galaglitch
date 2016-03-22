#include <stdint.h>
#include <iron/types.h>
#include "data_table.h"
#include "uthash.h"
#include "string_table.h"

#include <stdbool.h>
#include <iron/mem.h>
#include <iron/utils.h>
#include <stdio.h>

struct _string_table_entry{

  char * name;
  table_index index;
  UT_hash_handle hh;
};

typedef char * string;

int string_do_print(char * buffer, int size, string * data){
  if(*data == NULL)
    return snprintf(buffer, size,"0", *data);
  return snprintf(buffer, size,"\"%s\"", *data);
}

table_def * string_table_get_def(){
  static table_def def;
  if(def.cnt == 0){
    column_def columns[] = {COLUMN_DEF(string_table, entries, string)};
    def.columns = iron_clone(columns, sizeof(columns));
    def.total_size = sizeof(string_table);
    def.cnt = array_count(columns);
  }
  return &def;
}

table_index string_table_get(string_table * table, const char * str){
  string_table_entry * ent = NULL;
  HASH_FIND_STR(table->hash_table, str, ent);
  if(ent == NULL)
    return table_index_default;
  return ent->index;
}

table_index string_table_insert(string_table * table, const char * str){
  string_table_entry * ent = NULL;
  HASH_FIND_STR(table->hash_table, str, ent);
  if(ent != NULL)
    return ent->index;
  ent = alloc0(sizeof(string_table_entry));
  ent->name = iron_clone(str, strlen(str) + 1);
  ent->index = table_add_row(table);
  u64 stridx = table_raw_index(table, ent->index);
  table->entries[stridx] = ent->name;
  HASH_ADD_STR(table->hash_table, name, ent);
  return ent->index;
}

