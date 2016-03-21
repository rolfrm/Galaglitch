typedef struct{
  u64 index_data;
}table_index;

extern table_index table_index_default;
table_index table_index_new(u64 index, u8 _check);
#define TABLE_HEADER				\
  struct{					\
    u8 * index_check;				\
    u64 cnt;					\
    u64 * unused_indexes;			\
    u64 unused_index_cnt;			\
  };

typedef struct{
  TABLE_HEADER;
  u32 * type;
  u64 * data;
}data_table;

data_table * data_table_new();
void data_table_delete(data_table ** table);
table_index data_table_insert(data_table * table, u32 type, u64 data);
void data_table_remove(data_table * table, table_index t_index);
u64 data_table_index(data_table * table, table_index t_index);
u32 table_type_new();
#define TABLE_TYPE(X) (X ? X : (X = table_type_new()));
