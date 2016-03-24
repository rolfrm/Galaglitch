// depends on iron/types.h
typedef struct{
  union{
    u64 index_data;
    struct{
      u64 raw : 56;
      u8 check : 8;
    };
  };
}table_index;

extern table_index table_index_default;
table_index table_index_new(u64 index, u8 _check);

typedef struct{
  u32 size;
  u32 offset;
  int (* printer)(char * out, int size, void *);
  const char * name;
  const char * type_name;
}column_def;

typedef struct{
  column_def * columns;
  u32 cnt;
  u32 total_size;
}table_def;

typedef struct _table_header{					
  u8 * index_check;				
  u64 cnt;					
  u64 * unused_indexes;			
  u64 unused_index_cnt;
  table_def * def;
} table_header;

typedef struct{
  table_header header;
  u32 * type;
  union {
    u64 * data;
    table_index * index;
  };
}data_table;


table_header * _table_new(table_def * table_def);
#define table_new(type)(type *)_table_new(type ## _get_def())
void _table_delete(table_header ** table);
#define table_delete(table_loc) _table_delete((table_header **) table_loc)
table_index _table_add_row(table_header * table);
#define table_add_row(table) _table_add_row(&table->header)
void _table_remove(table_header * table, table_index t_index);
#define table_remove(table, index) _table_remove(&table->header, index)
// warning: this index is temporary. Use table_index for storage.
u64 _table_raw_index(table_header * table, table_index t_index);
#define table_raw_index(table, index) _table_raw_index(&table->header, index)

bool table_index_is_valid(table_index t_index);

u32 table_type_new();
#define TABLE_TYPE(X) (X ? X : (X = table_type_new()))
table_def * data_table_get_def();
column_def column_def_new(u32 offset, u32 size, const char * name, void * printer, const char * type_name);
#define COLUMN_DEF(type, member, membertype) \
  column_def_new(offsetof(type, member), sizeof(membertype), #member, membertype ## _do_print, #membertype)

#define table_lookup(table, column, index)(table->column + table_raw_index(table, index))
int float_do_print(char *, int size,  float *);
int u32_do_print(char *, int size, u32 *);
int u64_do_print(char *, int size, u64 *);
int u8_do_print(char * o, int size, u8 * v);
int int_do_print(char * o, int size, int * v);
int table_index_do_print(char * o, int size, table_index * t);
void _table_print(table_header * table);
#define table_print(table) _table_print(&table->header);
