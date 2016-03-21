typedef struct{
  u64 index_data;
}table_index;

typedef struct{
  u8 * index_check;
  u32 * type;
  u64 * data;
  
  u64 cnt;

  u64 * unused_indexes;
  u64 unused_index_cnt;
  
}data_table;

table_index data_table_insert_item(data_table * table, u32 type, u64 data);
data_table * data_table_new();
void data_table_delete(data_table ** table);
table_index data_table_insert_item(data_table * table, u32 type, u64 data);
void data_table_remove_item(data_table * table, table_index t_index);
void data_table_data(data_table * table, table_index t_index, u32 * out_type, u64 * out_data);
u32 table_type_new();
#define TABLE_TYPE(X) (X ? X : (X = table_type_new()));
