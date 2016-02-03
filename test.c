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
#include "game.h"

#include <iron/log.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
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


// mock
controller get_controller(){
  return (controller){0,0};
}

void print_models(game_models * models){
  vertex_list * verts = &models->verts;
  face_list * faces = &models->faces;
  for(size_t i = 0; i < verts->cnt; i++){
    logd("Vertex %i: %f %f %f\n", i, verts->x[i], verts->y[i], verts->z[i]);
  }
  for(size_t i = 0; i < faces->cnt; i++){
    logd("Face %i: %i %i %i\n", i, faces->v1[i], faces->v2[i], faces->v3[i]);
  }
}

void print_entities(game_entities * entities){
  for(size_t i = 0; i < entities->cnt; i++){
    logd("Entity %i: %i %i %f %f %f %f %f %i\n", i, entities->type[i], entities->vertex[i],
	 entities->x[i], entities->y[i], entities->dx[i], entities->dy[i], entities->a[i],
	 entities->model[i]);
    
  }
}
void add_vertex(vertex_list * verts, float x, float y, float z){
  list_push(verts->x, verts->cnt, x);
  list_push(verts->y, verts->cnt, y);
  list_push(verts->z, verts->cnt, z);
  verts->cnt++;
}

void add_edge(edge_list * edges, size_t f1, size_t f2){
  list_push(edges->f1, edges->cnt, f1);
  list_push(edges->f2, edges->cnt, f2);
  edges->cnt++;
}

void add_face(face_list * faces, int v1, int v2, int v3){
  list_push(faces->v1, faces->cnt, v1);
  list_push(faces->v2, faces->cnt, v2);
  list_push(faces->v3, faces->cnt, v3);
  faces->cnt += 1;
}

void gen_floor(game_floor * f){
  add_vertex(&f->vertexes, -2,-2,0);
  add_vertex(&f->vertexes, 2,-2,0);
  add_vertex(&f->vertexes, 2,2,0);
  add_vertex(&f->vertexes, -2,2,0);
  add_vertex(&f->vertexes, -2,6,0);
  add_vertex(&f->vertexes, 2,6,0);
  add_face(&f->faces, 0, 1, 2);
  add_face(&f->faces, 0, 3, 2);
  add_face(&f->faces, 4, 0, 1);
  add_face(&f->faces, 4, 5, 1);
}

void delete_game_data(game_data * gd){
  dealloc(gd->floor.vertexes.x);
  dealloc(gd->floor.vertexes.y);
  dealloc(gd->floor.vertexes.z);
  dealloc(gd->floor.faces.v1);
  dealloc(gd->floor.faces.v2);
  dealloc(gd->floor.faces.v3);
  memset(gd, 0, sizeof(*gd));
}

void load_mock_game_data(game_data * gd){
  vertex_list * verts = &gd->models.verts;
  face_list * faces = &gd->models.faces;
  // load a triangle model
  add_vertex(verts, -1,0,0);
  add_vertex(verts, 0,1,0);
  add_vertex(verts, 1,0,0);
  add_face(faces, 0,1,2);

  // load a game entity.
  game_entities * entities = &gd->entities;
  list_push(entities->type, entities->cnt, 0);
  list_push(entities->vertex, entities->cnt, 0);
  list_push(entities->x, entities->cnt, 0);
  list_push(entities->y, entities->cnt, 0);
  list_push(entities->dx, entities->cnt, 0.5);
  list_push(entities->dy, entities->cnt, 0.25);
  list_push(entities->a, entities->cnt, 0.25);
  list_push(entities->model, entities->cnt, 0);
  entities->cnt += 1;

  game_floor * floor = &gd->floor;
  gen_floor(floor);
  
}

bool test_main_loop(){
  game_data gd = {0};
  game_entities * entities = &gd.entities;
  load_mock_game_data(&gd);
  ASSERT(entities->x[0] == 0);
  

  controller ctrl ={0};
  float dt = 0.5;
  game_iteration(ctrl, dt, &gd);
  ASSERT(entities->x[0] == dt * 0.5);
  game_iteration(ctrl, dt, &gd);
  ASSERT(entities->x[0] == dt * 0.5 * 2);
  game_iteration(ctrl, dt, &gd);
  ASSERT(entities->x[0] == dt * 0.5 * 3);
  print_entities(entities);
  return TEST_SUCCESS;
}

bool test_graphics(){
  game_data gd = {0};
  load_mock_game_data(&gd);

  game_ui * rnd = game_ui_init();
  ASSERT(rnd != NULL);
  for(int i = 0; i < 10; i++){
    game_ui_update(rnd, &gd);
    iron_usleep(100000);
  }
  game_ui_deinit(&rnd);
  ASSERT(rnd == NULL);  
  return TEST_SUCCESS;
}

float circle_distance(float x, float y, float sx, float sy, float r){
  x -= sx;
  y -= sy;
  return sqrtf(x * x + y * y ) - r ;
}

float _mod(float v, int div){
  int rest = v / div;
  return v - rest * div;
}

float distance(float x, float y, void * distance_field){
 
  float d3 = -circle_distance(x,y, 200, 200, 150);
  
  if(d3 < 1.0)
    return d3;
  x = fmodf(x, 33) - 16;
  y = fmodf(y, 33) - 16;
  
  float d4 = circle_distance(x, y, 0, 0, 10);
  return MIN(d4, d3);
  //float d4 = circle_distance(x, y, 180, 180, 60);
  //if(d4 > 1) return MIN(d3, d4);
  
  //float d1 = circle_distance(x, y, 200, 220, 10);
  //float d2 = circle_distance(x, y, 200, 190, 10);
  //float d5 = circle_distance(x, y, 160, 160, 10);
  //return MIN(d3, MIN(d5, MIN(d1,d2)));
}

typedef struct {
  int width;
  int height;
  void * data;
}image;

void set_pixel_gray(image * img, int x, int y,u8 color){
  if(x >= 0 && y >= 0 && x < img->width && y < img->height){
    char * d = img->data;
    int offset = (x + y * img->width) * 3;
    d[offset] = color;
    d[offset + 1] = color;
    d[offset + 2] = color;
  }
}

void render_distance_field(image * img, float (* f)(float x, float y, void * userdata), void * userdata)
{
  int it = 0;
  for(int i =0 ; i < img->height; i++){
    for(int j = 0; j < img->width; j++){
      float d = f(j, i, userdata);
      if(d >= 1)
	continue;
      if(d < 0)
	d = 0.0;
      d = 1 - d;
      set_pixel_gray(img, j, i, d * 255);
      it++;    
    }
  }
  logd("it: %i\n", it);
}

typedef struct{
  float * angle;
  float * distance;
  int cnt;
  int capacity;
}trace_points;

void trace_points_add(trace_points * pts, float angle, float distance){
  if(pts->cnt == pts->capacity){
    int newcap = pts->cnt * 2 + 1;
    pts->angle = realloc(pts->angle, newcap * sizeof(float));
    pts->distance = realloc(pts->distance, newcap * 2 * sizeof(float));
    pts->capacity = newcap;
  }
  pts->angle[pts->cnt] = angle;
  pts->distance[pts->cnt] = distance;
  pts->cnt += 1; 
}

void trace_points_clear(trace_points * pts){
  pts->cnt = 0;
}

void trace_points_delete(trace_points * pts){
  dealloc(pts->angle);
  dealloc(pts->distance);
  memset(pts, 0, sizeof(trace_points));
}

void trace_distance_field_inner(image * img, trace_points * pts, float start_x, float start_y, float ang, float angle_sec, float d, float (* f)(float x, float y, void * userdata), void * userdata){
  float r_dist = sinf(angle_sec * 0.5);
  while(d < 400){
    float dx = sin(ang);
    float dy = cos(ang);
    float xoff = dx * d + start_x;
    float yoff = dy * d + start_y;
    float d2 = f(xoff, yoff, userdata);
    if(d2 < r_dist * d && angle_sec > 0.001){
      float ratio[] = {-0.5, 0.5};
      for(int i = 0; i < 2; i++){
	float angle = ang + angle_sec * ratio[i]; 
	trace_distance_field_inner(img, pts, start_x, start_y, angle, angle_sec * 0.5, d * 0.95, f, userdata);
      }
      return;
    }
    set_pixel_gray(img, (int)xoff, (int)yoff, 255);
    if(d2 < 1){
      trace_points_add(pts, ang, d + d2);
      return;
    }
    
    d += d2;
  }
}

void trace_distance_field(image * img, trace_points * pts, float start_x, float start_y,float (* f)(float x, float y, void * userdata), void * userdata){
  float angle = 2 * 3.14;
  float d = f(start_x, start_y, userdata);
  int cnt = 16;
  float angle_sec = (angle / (float)cnt);
  for(int i = 0; i < cnt; i++)
    trace_distance_field_inner(img, pts, start_x, start_y, angle_sec  * (float) i, angle_sec, d, f, userdata);
}

bool test_distance_field(){
  game_ui * rnd = game_ui_init();
  ASSERT(rnd != NULL);
  image img = {400, 400, alloc0(400 * 400 * 3)};
  trace_points pts = {0};
  for(int i = 0; i < 10000; i++){
    trace_points_clear(&pts);
    double phase = i * 0.01;
    double xpos, ypos;
    xpos = sin(phase) * 100 + 200;
    ypos = cos(phase) * 100 + 200;
    //game_ui_get_cursor_pos(rnd, &xpos, &ypos);
    //memset(img.data, 0, 400 * 400 * 3);
    u64 t1 = timestamp();
    //render_distance_field(&img, distance, NULL);
    trace_distance_field(&img, &pts, (int)xpos, 400 -(int)ypos, distance, NULL);
    logd("Trace points: %i\n", pts.cnt);
    //iron_usleep(100000);
    u64 t2 = timestamp();
    u64 dt = t2 - t1;
    logd("Render time: %i us\n", dt);
    game_ui_draw_image(rnd, img.data, img.width, img.height);
    iron_usleep(10000);
  }
  game_ui_deinit(&rnd);
  ASSERT(rnd == NULL);  
  return TEST_SUCCESS;
}

int main(){
  
  TEST(test_main_loop);
  //TEST(test_graphics);
  TEST(test_distance_field);
  return 0;
}
