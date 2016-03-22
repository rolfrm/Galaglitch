// depends on game_table, linmath, string_table

typedef struct{
  table_header header;
  // name in the string table.
  table_index * name;
  // data fields
  table_index * data1;
  table_index * data2;
  table_index * data3;
}entity_table;

table_def * entity_table_get_def();

typedef struct{
  table_header header;
  vec3 * loc;
  vec3 * size;
  vec3 * vel;
  float * mass;
}physics_table;

table_def * physics_table_get_def();

typedef struct{
  table_header header;
  table_index * name;
  int * width;
  int * height;
  u8 * format;
  u32 * gl_ref;
}gl_tex_table;
table_def * gl_tex_table_get_def();

typedef struct{
  table_header header;
  table_index * name;
  vec3 * loc;
  vec3 * size;

  table_index * texture;
  table_index * height_map;
}surface_table;

table_def * surface_table_get_def();

typedef struct{
  table_header header;
  int * width;
  int * height;
  u8 ** map;
}height_map_table;
table_def * height_map_table_get_def();

typedef struct{
  string_table * strings;
  entity_table * entities;
  data_table * data;
  physics_table * physics;
  surface_table * floors;
  gl_tex_table * textures;
}game_content;

game_content * init_game_content();
