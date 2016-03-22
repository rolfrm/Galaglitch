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

#include <iron/log.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include "data_table.h"
#include "string_table.h"
#include "game.h"

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
bool data_table_core_test(){
  TABLE_TYPE(TEST_TYPE_1);
  TABLE_TYPE(TEST_TYPE_2);

  data_table dtt;
  table_header *head = &dtt.header;
  TEST_ASSERT_EQUAL((void *) head, (void*) &dtt);
  data_table * loc = (data_table *) head;
  void * loc2 = &loc->type;
  size_t loc_offset = (size_t)(loc2 - (void *)loc);
  size_t loc_offset2 = (size_t)data_table_get_def()->columns[0].offset; 
  TEST_ASSERT_EQUAL(loc_offset, loc_offset2 );
  logd("Offsets: %i %i\n", loc_offset, loc_offset2);
  
  data_table * dt = table_new(data_table);
  table_index indexes[10];
  for(int i = 0; i < 10; i++){
    indexes[i] = table_add_row(dt);//data_table_insert(dt, TEST_TYPE_2, i * 10 + 50);
    u64 idx = table_raw_index(dt, indexes[i]);
    dt->type[idx] = TEST_TYPE_2;
    dt->data[idx] = i * 10 + 50;
  }

  for(u64 i = 0; i < 10; i++){
    u64 idx = table_raw_index(dt, indexes[i]);
    TEST_ASSERT_EQUAL(dt->data[idx], i * 10 + 50);
    TEST_ASSERT_EQUAL(dt->type[idx], TEST_TYPE_2);
  }
  TABLE_TYPE(TEST_TYPE_1);
  TABLE_TYPE(TEST_TYPE_2);
  for(int i = 0; i < 5; i++){
    table_remove(dt, indexes[i]);
  }
  TEST_ASSERT_EQUAL(dt->header.unused_index_cnt, 5);
  for(int i = 0; i < 5; i++){
    indexes[i] = table_add_row(dt);//, TEST_TYPE_1, i * 200 + 5);
    u64 idx = table_raw_index(dt, indexes[i]);
    dt->data[idx] = i * 200 + 5;
    dt->type[idx] = TEST_TYPE_1;
  }
  
  for(u64 i = 0; i < 5; i++){
    u64 idx = table_raw_index(dt, indexes[i]);
    TEST_ASSERT_EQUAL(dt->data[idx], i * 200 + 5);
    TEST_ASSERT_EQUAL(dt->type[idx], TEST_TYPE_1);
  }
  table_print(dt);
  table_delete(&dt);
  return TEST_SUCCESS;
}

typedef struct{
  table_header header;
  table_index * index0;
  table_index * index1;
  table_index * index2;
}span3_table;

int vec3_do_print(char * o, int size, vec3 *v){
  return snprintf(o, size, "(%f %f %f)", v->x, v->y, v->z);
}

table_def * span3_table_get_def(){
  static table_def def;
  if(def.cnt == 0){
    column_def columns[] = {COLUMN_DEF(span3_table, index0, table_index),
			    COLUMN_DEF(span3_table, index1, table_index),
			    COLUMN_DEF(span3_table, index2, table_index)};
			  
    def.total_size = sizeof(span3_table);
    def.columns = iron_clone(columns, sizeof(columns));
    def.cnt = array_count(columns);;
  }
  return &def;
}

table_def * physics_table_get_def(){
  static table_def def;
  if(def.cnt == 0){
    column_def columns[] = {COLUMN_DEF(physics_table, loc, vec3),
			    COLUMN_DEF(physics_table, size, vec3),
			    COLUMN_DEF(physics_table, vel, vec3),
			    COLUMN_DEF(physics_table, mass, float)};
    def.total_size = sizeof(physics_table);
    def.columns = iron_clone(columns, sizeof(columns));
    def.cnt = array_count(columns);
  }
  return &def;
}

typedef struct{
  table_header header;
  table_index * image_asset;
  table_index * name; 
}sprite_table;

table_def * sprite_table_get_def(){
  static table_def def;
  if(def.cnt == 0){
    column_def columns[] = {COLUMN_DEF(sprite_table, image_asset, table_index),
			    COLUMN_DEF(sprite_table, name, table_index)};
    def.columns = iron_clone(columns, sizeof(columns));
    def.cnt = array_count(columns);
    def.total_size = sizeof(sprite_table);
  }
  return &def;
}

bool span_table_test(){
  u64 TT_PLAYER = table_type_new();
  u64 TT_ENEMY = table_type_new();
  
  data_table * dt = table_new(data_table);
  span3_table * table = table_new(span3_table);
  physics_table * ptable = table_new(physics_table);
  COLUMN_DEF(physics_table, mass, float);
  {
    table_index i1 = table_add_row(dt);
    u64 player_index = table_raw_index(dt, i1);
    dt->type[player_index] = TT_PLAYER;
    dt->data[player_index] = 5;
    table_index player = table_add_row(table);
    table->index0[table_raw_index(table, player)] = i1;
    table_index p1 = table_add_row(ptable);
    table->index1[table_raw_index(table, player)] = p1;
    u64 pidx = table_raw_index(ptable,p1);
    ptable->mass[pidx] = 1.0;
    ptable->vel[pidx] = vec3_new(1,2,3);
  }

  {
    table_index i2 = table_add_row(dt);
    u64 enemy_index = table_raw_index(dt, i2);
    dt->type[enemy_index] = TT_ENEMY;
    dt->data[enemy_index] = 4;
    table_index enemy = table_add_row(table);
    table_index p2 = table_add_row(ptable);
    table->index0[table_raw_index(table, enemy)] = i2;
    table->index1[table_raw_index(table, enemy)] = p2;
    ptable->mass[table_raw_index(ptable, p2)] = 2.0;
    ptable->vel[table_raw_index(ptable, p2)] = vec3_new(0,0.2,0.1);
  }
  table_print(table);
  table_print(ptable);
  table_print(dt);
  for(size_t i = 1; i < table->header.cnt; i++){
    
    u64 idx = table_raw_index(dt, table->index0[i]);
    u64 idx2 = table_raw_index(ptable, table->index1[i]);
    TEST_ASSERT(ptable->mass[idx2] > 0 && ptable->mass[idx] > 0);
  }
  for(int j = 0; j < 5; j++){
    table_print(ptable);
    for(size_t i = 1; i < ptable->header.cnt; i++){
      ptable->loc[i] = vec3_add(ptable->loc[i], ptable->vel[i]);
    }
  }
  table_print(table);
  table_print(ptable);
  table_print(dt);
  table_delete(&ptable);
  table_delete(&dt);
  table_delete(&table);
  
  return TEST_SUCCESS;
}

bool string_table_test(){
  string_table * table = table_new(string_table);
  string_table_insert(table, "Hello?");
  string_table_insert(table, "Hello?");
  string_table_insert(table, "Helloma?");
  string_table_insert(table, "Red?");
  string_table_insert(table, "Blue?");
  string_table_insert(table, "Red?");
  string_table_insert(table, "Green?");
  string_table_insert(table, "Green!");
  string_table_insert(table, "Purple");
  string_table_insert(table, "purple");
  string_table_insert(table, "purple-grayish");
  string_table_insert(table, "lavender");
  string_table_insert(table, "alice-blue");
  table_index idx = string_table_get(table, "Blue?");
  ASSERT(idx.index_data != 0);
  table_print(table);
  return TEST_SUCCESS;
}

enum image_format{
  FORMAT_NONE = 0,
  FORMAT_GRAY = 1,
  FORMAT_RG = 2,
  FORMAT_RGB = 3,
  FORMAT_RGBA = 4
};
#include <iron/fileio.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "shader_utils.h"
//nclude <GL/glew.h>
void * stbi_load(const char * path, int * width, int * height, int * components, int * req_components);
int load_png_as_texture(const char * path, u8 * in_out_format, u32 * out_glref, int * out_width, int * out_height){

  int fmt = *in_out_format;
  void * buf = stbi_load(path, out_width, out_height, &fmt, fmt == 0 ? 0 : &fmt); 
  *in_out_format = fmt;
  glGenTextures(1, out_glref);
  glBindTexture(GL_TEXTURE_2D, *out_glref);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  int internal_format = GL_RGB;
  switch(fmt){
  case 1:
    internal_format = GL_RED;
    break;
  case 2:
    internal_format = GL_RG;
    break;
  case 3:
    internal_format = GL_RGB;
    break;
  case 4:
    internal_format = GL_RGBA;
    break;
  default:
    ERROR("Invalid format: %i\n", fmt);
    return 1;
  }
  
  glTexImage2D(GL_TEXTURE_2D, 0,internal_format, *out_width, *out_height, 0, internal_format, GL_UNSIGNED_BYTE, buf);
  glBindTexture(GL_TEXTURE_2D, 0);
  dealloc(buf);
  return 0;
}



bool game_content_test(){
  glfwInit();
  GLFWwindow * win = glfwCreateWindow(200, 200, "Galaglitch", NULL, NULL);
  glfwMakeContextCurrent(win);
  glewInit();
  size_t s1_vert_size, s1_frag_size;
  char * s1_vert = read_file_to_buffer("assets/s1.vert", &s1_vert_size);
  char * s1_frag = read_file_to_buffer("assets/s1.frag", &s1_frag_size);
  int shader = load_simple_shader(s1_vert, s1_vert_size, s1_frag, s1_frag_size);
  TEST_ASSERT(shader != -1);
  u32 vbo;
  {
    int pointcnt = 32;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 2 * (1 + pointcnt) * sizeof(float), NULL, GL_STREAM_DRAW);
    float * buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    for(int i = 0; i < pointcnt; i++){
      buffer[i*2] = sin(2.0 * M_PI * ((float) i) / pointcnt);
      buffer[i*2 + 1] = cos(2.0 * M_PI * ((float) i) / pointcnt);
    }
    buffer[pointcnt * 2] = buffer[0];
    buffer[pointcnt * 2 + 1] = buffer[1];
    glUnmapBuffer(GL_ARRAY_BUFFER);
  }
  TEST_ASSERT(vbo > 0);
  game_content * content = init_game_content();
  TEST_ASSERT(content != NULL);
  {
    table_index idx = table_add_row(content->entities);
    content->entities->name[idx.raw] = string_table_insert(content->strings, "Player");
    { // add physics body
      table_index data1 = table_add_row(content->data);
      content->data->type[data1.raw] = TABLE_TYPE(physics_type);
      table_index p = table_add_row(content->physics);
      content->data->data[data1.raw] = p.index_data;
      content->entities->data1[idx.raw] = data1;
      content->physics->loc[p.raw] = vec3_new(0.5, 0.5, 0.0);
      content->physics->vel[p.raw] = vec3_new(0.1, 0.1, 0.0);
    }
    { // add sprite
      const char * path = "assets/pl1.png";
      table_index tex = table_add_row(content->textures);
      table_index name = string_table_insert(content->strings, path);
      content->textures->name[tex.raw] = name;
      int load_status = load_png_as_texture(path, content->textures->format + tex.raw,
					    content->textures->gl_ref + tex.raw,
					    content->textures->width + tex.raw, content->textures->height + tex.raw);
      ASSERT(load_status == 0);
    }

  }
  {
    table_index idx = table_add_row(content->entities);
    content->entities->name[idx.raw] = string_table_insert(content->strings, "Enemy");
    table_index data1 = table_add_row(content->data);
    content->data->type[data1.raw] = TABLE_TYPE(physics_type);
    content->data->data[data1.raw] = table_add_row(content->physics).index_data;
    content->entities->data1[idx.raw] = data1;
  }

  for(int i = 0; i < 1000; i++){
    for(u64 j = 1; j < content->physics->header.cnt; j++){
      content->physics->loc[j] = vec3_add(content->physics->loc[j], content->physics->vel[j]);
      content->physics->vel[j] = vec3_scale(content->physics->vel[j], 0.95);
    }
    u64 t1 = timestamp();

    glClearColor(0.1,0.1,0.1,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glUseProgram(shader);
    for(u32 j = 0 ; j < content->entities->header.cnt; j++){
      table_index idx = content->entities->data1[j];
      if(!table_index_is_valid(idx)){
	continue;
      }
      u64 k = table_raw_index(content->data, idx);
      if(content->data->type[k] == physics_type){
	table_index ti;
	ti.index_data = content->data->data[k];
	if(!table_index_is_valid(ti))
	  continue;
	u64 k2 = table_raw_index(content->physics, ti);
	vec3 loc = content->physics->loc[k2];
	glUniform2f(glGetUniformLocation(shader, "offset"), loc.x, loc.y);
	glDrawArrays(GL_LINE_LOOP, 0, 33);
      }
    }
    glfwSwapBuffers(win);
    u64 t2 = timestamp();
    u64 dt = t2 - t1;
    if((i%10) == 0)
      logd("dt: %fms\n", (float)dt * 0.001);
    iron_usleep(10000);
  }
  table_print(content->entities);
  table_print(content->strings);
  table_print(content->physics);
  table_print(content->textures);
  return TEST_SUCCESS;
}

int main(){
  TEST(data_table_core_test);
  TEST(span_table_test);
  TEST(string_table_test);
  TEST(game_content_test);
  //TEST(test_line_segment);
  //TEST(test_main_loop);
  //TEST(test_graphics);
  //TEST(test_distance_field);
  return 0;
}
