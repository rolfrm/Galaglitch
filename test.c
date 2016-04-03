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
#include "image.h"
#include "vr_video_test.h"

#include <iron/fileio.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "shader_utils.h"

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

//nclude <GL/glew.h>
void * stbi_load(const char * path, int * width, int * height, int * components, int * req_components);
int load_png_as_texture(const char * path, u8 * in_out_format, u32 * out_glref, int * out_width, int * out_height){

  int fmt = *in_out_format;
  void * buf = stbi_load(path, out_width, out_height, &fmt, fmt == 0 ? 0 : &fmt); 
  *in_out_format = fmt;
  glGenTextures(1, out_glref);
  glBindTexture(GL_TEXTURE_2D, *out_glref);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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
      table_index data2 = table_add_row(content->data);
      content->data->data[data2.raw] = tex.index_data;
      content->data->type[data2.raw] = TABLE_TYPE(sprite_type);
      content->entities->data2[idx.raw] = data2;
    }

  }
  {
    table_index idx = table_add_row(content->entities);
    content->entities->name[idx.raw] = string_table_insert(content->strings, "Enemy");
    table_index data1 = table_add_row(content->data);
    content->data->type[data1.raw] = TABLE_TYPE(physics_type);
    table_index p = table_add_row(content->physics);
    content->data->data[data1.raw] = p.index_data;
    content->entities->data1[idx.raw] = data1;
    //table_insert2(content->physics, vel, loc, vec3_new(-0.2, 0.2, 0.0), vec3_new(0.2,2.9,0.0));
    content->physics->vel[p.raw] = vec3_new(-0.2 ,0.2, 0.0);
    content->physics->loc[p.raw] = vec3_new(0.2,2.9,0.0);
  }

  for(int i = 0; i < 10000; i++){
    int err = glGetError();
    ASSERT(err == 0);
    for(u64 j = 1; j < content->physics->header.cnt; j++){
      content->physics->loc[j] = vec3_add(content->physics->loc[j], content->physics->vel[j]);
      content->physics->vel[j] = vec3_scale(content->physics->vel[j], 0.95);
    }
    u64 t1 = timestamp();

    glClearColor(0.1,0.1,0.1,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
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
	table_index ti = content->data->index[k];
	if(!table_index_is_valid(ti))
	  continue;

	table_index idx2 = content->entities->data2[j];
	if(table_index_is_valid(idx2)){
	  if(*table_lookup(content->data, type, idx2) == TABLE_TYPE(sprite_type)){
	    table_index sprite_index = *table_lookup(content->data, index, idx2);
	    glActiveTexture(GL_TEXTURE0 + 0);
	    glBindTexture(GL_TEXTURE_2D, *table_lookup(content->textures, gl_ref, sprite_index));
	    glUniform1i(glGetUniformLocation(shader, "tex"), 0);
	  }
	}
	vec3 loc = *table_lookup(content->physics, loc, ti);
	
	glUniform2f(glGetUniformLocation(shader, "offset"), loc.x, loc.y);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 33);
      }
    }
    glfwSwapBuffers(win);
    u64 t2 = timestamp();
    u64 dt = t2 - t1;
    if((i%10) == 0)
      logd("dt: %fms\n", (float)dt * 0.001);
    iron_usleep(10000);
  }
  table_print(content->data);
  table_print(content->entities);
  table_print(content->strings);
  table_print(content->physics);
  table_print(content->textures);
  return TEST_SUCCESS;
}
typedef struct{
  int color;
  int child_config;
}octree_data;

vec3 vec3_less(vec3 a, vec3 b){
  return vec3_new(a.x < b.x, a.y < b.y, a.z < b.z);
}

vec3 vec3_select(vec3 a, vec3 b, vec3 mask){
  return vec3_new(mask.x == 0.0f ? b.x : a.x,
		  mask.y == 0.0f ? b.y : a.y,
		  mask.z == 0.0f ? b.z : a.z);
}

vec3 vec3_min(vec3 a, vec3 b){
  return vec3_new(a.x < b.x ? a.x : b.x,
		  a.y < b.y ? a.y : b.y,
		  a.z < b.z ? a.z : b.z);
}

float vec3_min_scalar(vec3 a){
  return MIN(a.x, MIN(a.y, a.z));
}

float vec3_max_scalar(vec3 a){
  return MAX(a.x, MAX(a.y, a.z));
}
#define FSWAP(a,b){ float tmp = b; b = a; a = tmp;}
vec3 vec3_sort(vec3 a){
  if(a.z < a.x)
    FSWAP(a.x,a.z);
  if(a.x > a.y)
    FSWAP(a.x, a.y);
  if(a.y > a.z)
    FSWAP(a.z, a.y);
  
  return a;
}

typedef struct{
  u8 * jump_encoding;
  size_t cnt;
  size_t capacity;
}oct_data;
#include "immintrin.h"
#include "nmmintrin.h"

size_t measure_oct_data2(u8 *depth_buffer, u8 * _child_info){
  u8 * child_info = _child_info;
  size_t stack_ptr = 0;
  size_t stack[256];
  u8 conf = *child_info;
  stack[stack_ptr] = _mm_popcnt_u32(conf);
  size_t max_depth = 0;
  do{
    while(stack[stack_ptr] > 0){
      stack[stack_ptr]--;
      //*depth_buffer = stack_ptr;
      logd("offset: %i %i\n", stack_ptr, child_info - _child_info);   
      *depth_buffer = MAX(*depth_buffer, stack_ptr);      
   
      child_info += 1;
      depth_buffer += 1;

      u8 conf = *child_info;
      if(conf > 0){
	stack_ptr += 1;
	stack[stack_ptr] = _mm_popcnt_u32(conf);
      }else{
	*depth_buffer = MAX(*depth_buffer, stack_ptr + 1);
      }
    }
    if(stack_ptr == 0)
      break;
    if(stack_ptr > max_depth)
      max_depth = stack_ptr;
    stack_ptr -= 1;
  }while(true);
  
  return max_depth + 1;
}

// measure the depth of each node in the tree.
// only supports depth up to 255. Should be enough.
// 2^255 in mm = 10 ^ 73 m.
// The size of the known universe is only ~10 ^ 27 m

size_t measure_oct_data(u8 * depth_buffer, u8 * child_info){
  u8 conf = *child_info;
  int bits = _mm_popcnt_u32(conf);
  size_t offset = 1;
  u8 depth = 0;
  for(int i = 0; i < bits; i++){
    size_t of = measure_oct_data(depth_buffer + offset, child_info + offset);
    depth = MAX(depth, depth_buffer[offset]);
    offset += of;
  }
  if(bits > 0)
    *depth_buffer = depth + 1;
  return offset;
}

typedef struct{
  u8 * oct_data;
  size_t size;
}oct_data2;

typedef struct{
  // offset into the initial data structure
  size_t offset1;
  // offset into the one with jump trees
  size_t offset2;
}size_tuple;

size_tuple load_oct_data(oct_data2 * oct, size_t offset, u8 * child_info, u8 * depth){
  size_tuple out_sizes = {1, 1};
  u8 conf = *child_info;
  
  if(conf == 0){
    ASSERT(0 == oct->oct_data[offset]);
    return out_sizes;
  }
  
  int bits = _mm_popcnt_u32(conf);

  if(bits == 1){
    
    oct->oct_data[offset] = conf;
    size_tuple in_sizes = load_oct_data(oct, offset + 1,
					child_info + 1,
					depth + 1 );
    out_sizes.offset1 += in_sizes.offset1;
    out_sizes.offset2 += in_sizes.offset2;
    return out_sizes;
  }
  u8 jmp_size = 0;
    // jump table needed.
  if(*depth < 7)
    jmp_size = 0;
  else if(*depth < 15)
    jmp_size = 1;
  else if(*depth < 32)
    jmp_size = 2;
  else if(*depth < 64)
    jmp_size = 3;
  else ERROR("Depth not supported: %i", *depth);
	
  oct->oct_data[offset] = conf;
  oct->oct_data[offset + 1] = jmp_size;
  out_sizes.offset2 = 2 + (1 << jmp_size) * (bits - 1);
  size_t child_start = out_sizes.offset2;
  size_t size_headers = offset + 2;
  out_sizes.offset1 = 1;
  for(int i = 0; i < bits; i++){

    if(i > 0){
      u64 rel_offset = out_sizes.offset2 - child_start;
      logd("jmp: %i\n", size_headers + (i - 1) * (1 << jmp_size));
      memcpy(oct->oct_data + size_headers + (i - 1) * (1 << jmp_size), &rel_offset, (1 << jmp_size));
    }

    size_tuple in_sizes = load_oct_data(oct, offset + out_sizes.offset2,
					child_info + out_sizes.offset1,
					depth + out_sizes.offset1 );
    out_sizes.offset1 += in_sizes.offset1;
    out_sizes.offset2 += in_sizes.offset2;
  }
  
  return out_sizes;
}

size_t verify_oct_data(u8 * oct, size_t depth){
  u8 * octinit = oct;
  u8 header = *(oct++);
  logd("Header: %i %i\n", header, depth);
  int bits = _mm_popcnt_u32(header);
  if(bits == 0){
    return 1;
  }
  if(bits == 1){
    return 1 + verify_oct_data(oct, depth + 1);
  }
  
  u64 jumps[bits];
  u8 jump_type = *(oct++);
  int ji = bits - 1;
  jumps[0] = 0;
  if(jump_type == 0){
    for(int i = 0; i < ji;i++){
      jumps[i + 1] = oct[i];
    }
    oct += sizeof(u8) * ji;
  }else if(jump_type == 1){
    for(int i = 0; i < ji;i++){
      jumps[i + 1] = ((u16 *) oct)[i];
    }
    oct += sizeof(u16) * ji;
  }else if(jump_type == 2){
    for(int i = 0; i < ji;i++){
      jumps[i + 1] = ((u32 *) oct)[i];
    }
    oct += sizeof(u32) * ji;
  }else if(jump_type == 3){
    for(int i = 0; i < ji;i++){
      jumps[i + 1] = ((u64 *) oct)[i];
    }
    oct += sizeof(u64) * ji;
  }else{
    ERROR("Invalid jump type: %i", jump_type);
  }
  logd("jump type: %i\n", jump_type);
  size_t total = oct - octinit;
  for(int i = 0; i < bits; i++){
    printf("jump %i %i\n", jumps[i], total);
    if(i > 0)
      ASSERT(jumps[i] > 0);
    total += verify_oct_data(oct + jumps[i], depth + 1);
  }
  return total;
}

u8 * move_to_child(u8 * oct, int childid){
  u8 header = *(oct++);
  ASSERT(header & 1 >> childid);
  int bits = _mm_popcnt_u32(header);
  if(bits == 1)
    return oct;
  u8 jump_type = *(oct++);

  int jmpidx = 0;
  for(int i = 0; i < 8;i++){
    if(i == childid)
      break;
    if((i >> 1) & header)
      jmpidx++;
  }
  if(jmpidx == 0)
    return oct;
  if(jump_type == 0){
    u8 * v = oct + jmpidx * 1;
    oct += *v;
  }else if(jump_type == 1){
    u16 * v = (u16 *)(oct + jmpidx * 2);
    oct += *v;
  }else if(jump_type == 2){
    u32 * v = (u32 *)(oct + jmpidx * 2);
    oct += *v;
  }else if(jump_type == 3){
    u64 * v = (u64 *)(oct + jmpidx * 2);
    oct += *v;
  }else{
    ERROR("Invalid jump type: %i", jump_type);
  }
  
  return oct;
}

float cast_ray(u8 * oct, u8 * colors, vec3 ray_orig, vec3 ray_dir)
{
  UNUSED(colors);
  vec3 vec3_ones = vec3_new(1,1,1);
  vec3 vec3_half = vec3_new(0.5, 0.5, 0.5);
  vec3 vec3_zeros = vec3_new(0, 0, 0);
  
  vec3 d1 = vec3_div(vec3_sub(vec3_zeros, ray_orig), ray_dir);
  float d = vec3_max_scalar(d1);
  ray_orig = vec3_add(vec3_scale(ray_dir, d), ray_orig);
  vec3_print(ray_orig);
  int child_conf = *oct; //child_info[cell_stack[stack_index]];
  if(child_conf != 0){
    // at this point the ray is at some the lower bounds of the cube.
    int ex = ray_orig.x >= 0.5;
    int ey = ray_orig.y >= 0.5;
    int ez = ray_orig.z >= 0.5;
    int cell = ex | ey >> 1 | ez >> 2;
    logd("Cell: %i\n", cell);
    int cell_id = cell;//indexes[cell];
    if(child_conf & (1 >> cell_id)){
      u8 * c = move_to_child(oct, cell);
      cast_ray(c, colors, ray_orig, ray_dir);
    }
    vec3 d2 = vec3_div(vec3_sub(vec3_half, ray_orig), ray_dir);
    float d_2 = vec3_min_scalar(d2);
    ray_orig = vec3_add(vec3_scale(ray_dir, d_2), ray_orig);
    vec3_print(ray_orig);logd("\n");
  }
    //vec3_print(d1);vec3_print(d2);logd("\n");
    //vec3_print(d1_);vec3_print(d2_);logd("\n");
    //logd("min: ");vec3_print(d);logd("\n");
  vec3 test1 = vec3_less(ray_orig, vec3_zeros);
  vec3 test2 = vec3_less(vec3_ones, ray_orig);
  vec3_print(test1);vec3_print(test2);logd("\n");
  //bool no_col = vec3_any( vec3_or(test1, test2) );
  return 0;
}

bool gpu_octree_test(){

  i32 color_data[] = {0, 0xFFFFFFFF, 0, 0xFF00FFFF};
  u8 child_info[] = {3,0,1,3,0,1,1,1,1 + 2 + 4,1,1,3,1,0,1,1,0,1,1 ,1 , 1, 1, 0, 1, 1, 1, 1, 1, 0};
  u8 depth_info[array_count(child_info)];
  memset(depth_info, 0, sizeof(depth_info));

  size_t max_depth = measure_oct_data(depth_info, child_info);
  oct_data2 d;
  d.oct_data = alloc0(max_depth * 3);
  d.size = max_depth;

  for(u32 i = 0; i < array_count(child_info); i++){
    logd("%i %i\n", i, depth_info[i]);
  }
  logd("Max depth: %i\n", max_depth);
  size_tuple t = load_oct_data(&d, 0, child_info, depth_info);
  logd("Size: %i %i\n", t.offset1, t.offset2);
  
  for(int i = 0; i < 18;i++){
    logd("%x ", d.oct_data[i]);
  }
  logd("\n");
  size_t verification = verify_oct_data(d.oct_data, 0);
  logd("Verification: %i\n", verification);
  //u8 indexes[] = {0,1,2,3,4,5,6,7};
  
  //vec3 vec3_ones = vec3_new(1,1,1);
  printf("octree len: %i %i\n", array_count(color_data), array_count(child_info));
  //vec3 octree_orig = vec3_new(0,0,0);
  //vec3 octree_size = vec3_new(1,1,1);
  //vec3 octree_upper = vec3_add(octree_orig, octree_size);
  vec3 ray_orig = vec3_new(-0.1,-0.2,-0.1);
  vec3 ray_dir = vec3_normalize(vec3_new(1,2,1));
  float ds = cast_ray(d.oct_data, NULL, ray_orig, ray_dir);
  logd("fin %f\n", ds);
  // to simplify iterations i try to make sure ray dirs are always positive. Is this possible?
  /*if(ray_dir.x < 0){
    ray_orig.x = 1.0 - ray_orig.x;
    ray_dir.x = -ray_dir.x;
    SWAP(indexes[0], indexes[1]);
    SWAP(indexes[2], indexes[3]);
    SWAP(indexes[4], indexes[5]);
    SWAP(indexes[6], indexes[7]);
  }
  if(ray_dir.y < 0){
    ray_orig.y = 1.0 - ray_orig.y;
    ray_dir.y = -ray_dir.y;
    SWAP(indexes[0], indexes[2]);
    SWAP(indexes[1], indexes[3]);
    SWAP(indexes[4], indexes[6]);
    SWAP(indexes[5], indexes[7]);
  }
  if(ray_dir.z < 0){
    ray_orig.z = 1.0 - ray_orig.z;
    ray_dir.z = -ray_dir.z;
    SWAP(indexes[0], indexes[4]);
    SWAP(indexes[1], indexes[5]);
    SWAP(indexes[2], indexes[6]);
    SWAP(indexes[3], indexes[7]);
  }
  
  int jmp_stk_idx = 0;
  int stack_index = 0;
  int cell_stack[32];
  int jump_stack[32];
  cell_stack[0] = 0;
  jump_stack[0] = 1;
  //int ray_color = 0;
  while(true){
    //union{
    //  int color;
    //  struct { u8 r,g,b,a;};
    //  };
    
    vec3 d1 = vec3_div(vec3_sub(vec3_zeros, ray_orig), ray_dir);
    float d = vec3_max_scalar(d1);
    ray_orig = vec3_add(vec3_scale(ray_dir, d), ray_orig);
    vec3_print(ray_orig);
    int child_conf = child_info[cell_stack[stack_index]];
    if(child_conf != 0){
      // at this point the ray is at some the lower bounds of the cube.
      int ex = ray_orig.x >= 0.5;
      int ey = ray_orig.y >= 0.5;
      int ez = ray_orig.z >= 0.5;
      int cell = ex | ey << 1 | ez << 2;
      logd("Cell: %i\n", cell);
      int cell_id = indexes[cell];
      if(child_conf & (1 >> cell_id)){
	
      }
      vec3 d2 = vec3_div(vec3_sub(vec3_half, ray_orig), ray_dir);
      float d_2 = vec3_min_scalar(d2);
      ray_orig = vec3_add(vec3_scale(ray_dir, d_2), ray_orig);
      vec3_print(ray_orig);logd("\n");
    }
    //vec3_print(d1);vec3_print(d2);logd("\n");
    //vec3_print(d1_);vec3_print(d2_);logd("\n");
    //logd("min: ");vec3_print(d);logd("\n");
    vec3 test1 = vec3_less(ray_orig, vec3_zeros);
    vec3 test2 = vec3_less(vec3_ones, ray_orig);
    bool no_col = vec3_any( vec3_or(test1, test2) );
    
    if(!no_col){
      int color = color_data[cell_stack[stack_index]];
      ray_color = color;
      if(ray_color.a == 0xFF){
	break;
      }
      }
    break;
    }*/
  return TEST_SUCCESS;
}
bool vr_video_test();
//bool test_distance_field();
int main(int argc, const char ** argv){
  
  set_test_opt(argv,argc);
  TEST(data_table_core_test);
  TEST(span_table_test);
  TEST(string_table_test);
  //TEST(game_content_test);
  TEST(gpu_octree_test);
  TEST(vr_video_test);
  //TEST(test_line_segment);
  //TEST(test_main_loop);
  //TEST(test_graphics);
  //TEST(test_distance_field);
  return 0;
}
