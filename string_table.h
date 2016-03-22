typedef struct _string_table_entry string_table_entry;

typedef struct{
  table_header header;  
  char ** entries;
  string_table_entry * hash_table;
}string_table;

table_index string_table_get(string_table * table, const char * str);
table_def * string_table_get_def();
table_index string_table_get(string_table * table, const char * str);
table_index string_table_insert(string_table * table, const char * str);
