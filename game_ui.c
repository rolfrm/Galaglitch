#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include<math.h>
#include <iron/types.h>
#include <iron/mem.h>
#include <iron/fileio.h>
#include <iron/log.h>
#include <iron/utils.h>
#include <iron/time.h>
#include <iron/linmath.h>
#include "game.h"
#include "hash.h"
#include "shader_utils.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <GL/gl.h>

#define assert_no_glerr() { int error = glGetError();  if(error != 0) ERROR("GL Error %i: %s\n", error, glewGetErrorString(error));}

typedef struct {
  int program;
  int vert_attr;
}s1;

typedef struct{
  u32 program;
  int vert_attr;
  int offset_uniform;
  int scale_uniform;
  int falloff_uniform;
  int color_uniform;
}s2;

typedef struct{
  u32 vertex_buffer;
  int vertex_cnt;
  u32 face_buffer;
  int face_cnt;
  u64 hash;
}ui_model;

typedef struct{
  u32 vertex_buffer;
  int cnt;
  int capacity;
  u64 hash;
}angle_model;

struct _game_ui{
  GLFWwindow * window;
  hashstate * hashstate;
  ui_model floor_model;
  s1 shader1;

  // testing
  s2 shader2;
  angle_model angle_model;
};

static void load_s1(s1 * s){
  char * vert_code = read_file_to_string("resources/s1.vert");
  char * frag_code = read_file_to_string("resources/s1.frag");
  ASSERT(vert_code != NULL);
  ASSERT(frag_code != NULL);
  i32 prog = load_simple_shader(vert_code, strlen(vert_code), frag_code, strlen(frag_code));
  s->program = prog;
  s->vert_attr = 0;
  dealloc(vert_code);
  dealloc(frag_code);
}

static void load_s2(s2 * s){
  char * vert_code = read_file_to_string("resources/s2.vert");
  char * frag_code = read_file_to_string("resources/s2.frag");
  ASSERT(vert_code != NULL);
  ASSERT(frag_code != NULL);
  i32 prog = load_simple_shader(vert_code, strlen(vert_code), frag_code, strlen(frag_code));
  s->program = prog;
  s->vert_attr = 0;
  s->offset_uniform = glGetUniformLocation(s->program, "offset");
  s->scale_uniform = glGetUniformLocation(s->program, "scale");
  s->falloff_uniform = glGetUniformLocation(s->program, "falloff");
  s->color_uniform = glGetUniformLocation(s->program, "in_color");
  dealloc(vert_code);
  dealloc(frag_code);
}

/*static u64 get_model_hash(hashstate * hashstate, vertex_list verts, face_list faces){
  hashstate_reset(hashstate);
  hashstate_update(hashstate, verts.x, sizeof(*verts.x) * verts.cnt);
  hashstate_update(hashstate, verts.y, sizeof(*verts.y) * verts.cnt);
  hashstate_update(hashstate, verts.z, sizeof(*verts.z) * verts.cnt);
  hashstate_update(hashstate, faces.v1, sizeof(*faces.v1) * faces.cnt);
  hashstate_update(hashstate, faces.v2, sizeof(*faces.v2) * faces.cnt);
  hashstate_update(hashstate, faces.v3, sizeof(*faces.v3) * faces.cnt);
  return hashstate_digest(hashstate);
  }*/

// GL context dependent
/*static void load_model(game_ui * ui, ui_model * model, vertex_list verts, face_list faces){
  
  if(verts.cnt == 0) return;
  if(faces.cnt == 0) return;
  u64 model_hash = get_model_hash(ui->hashstate, verts, faces);
  if(model_hash == model->hash) return;
  logd("LOADING..\n");
  if(model->vertex_cnt == 0){
    u32 buffers[2];
    glGenBuffers(2, buffers);
    model->vertex_buffer = buffers[0];
    model->face_buffer = buffers[1];
  }
  { // load vertex buffer (interleaved)
    glBindBuffer(GL_ARRAY_BUFFER, model->vertex_buffer);
    size_t vsize = verts.cnt * sizeof(*verts.x);
    float * buffer = alloc0(vsize * 3);
    for(int i = 0; i < verts.cnt; i++){
      buffer[i * 3] = verts.x[i];
      buffer[i * 3 + 1] = verts.y[i];
      buffer[i * 3 + 2] = verts.z[i];
    }

    glBufferData(GL_ARRAY_BUFFER, vsize * 3, buffer, GL_STATIC_DRAW);
    dealloc(buffer);
  }
  { // load element buffer (interleaved)
    size_t vsize = faces.cnt * sizeof(*faces.v1);
    u32 * buffer = alloc0(vsize * 3);
    for(int i = 0; i < faces.cnt; i++){
      buffer[i * 3] = faces.v1[i];
      buffer[i * 3 + 1] = faces.v2[i];
      buffer[i * 3 + 2] = faces.v3[i];
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->face_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vsize * 3, buffer, GL_STATIC_DRAW);
    dealloc(buffer);
  }
  model->hash = model_hash;
  model->vertex_cnt = verts.cnt;
  model->face_cnt = faces.cnt;
  }*/

static void load_angle_model(game_ui * ui, angle_model * model, float * angles, float * distances, int cnt){
  UNUSED(ui);
  int cnt2 = cnt + 3;
  if(model->vertex_buffer == 0){
    glGenBuffers(1, &model->vertex_buffer);
  }
  glBindBuffer(GL_ARRAY_BUFFER, model->vertex_buffer);
  if(cnt2 > model->capacity){
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * cnt2, NULL, GL_STREAM_DRAW);
    model->capacity = cnt2;
  }
  model->cnt = cnt2;
  float * data = glMapBufferRange(GL_ARRAY_BUFFER, 0, cnt2 * 2 * sizeof(float),
				  GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  ASSERT(data != NULL);
  if(cnt2 == 3){
    memset(data, 0, sizeof(float) * cnt2 * 3);
    
  }
  else{
    data[0] = 0;
    data[1] = 0;//distances[0];
    data += 2;
    data[0] = 0;
    data[1] = distances[0];
    data += 2;
    for(int i = 0; i < cnt; i++){
      data[i*2] = angles[i];
      data[i*2 + 1] = distances[i];
    }
    data[cnt * 2] = 0;
    data[cnt* 2 + 1] = distances[0];
  }
  ASSERT(GL_TRUE == glUnmapBuffer(GL_ARRAY_BUFFER));
}


static void draw_model(s1 shader, ui_model model){
  ASSERT(model.vertex_cnt > 0);
  glBindBuffer(GL_ARRAY_BUFFER, model.vertex_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.face_buffer);
  glEnableClientState(GL_VERTEX_ARRAY);
  assert_no_glerr();
  glVertexPointer(3, GL_FLOAT, 0, 0);
  glVertexAttribPointer(shader.vert_attr, 3, GL_FLOAT, GL_FALSE, 0, 0);
  assert_no_glerr();
  glDrawElements(GL_TRIANGLES, model.face_cnt * 3, GL_UNSIGNED_INT, 0);
  assert_no_glerr();
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDisableClientState(GL_VERTEX_ARRAY);  
}



void game_ui_update(game_ui * ui, const game_data * gd){
  UNUSED(gd);
  glfwMakeContextCurrent(ui->window);
  glUseProgram(ui->shader1.program);
  assert_no_glerr();
  //load_model(ui, &ui->floor_model, gd->floor.vertexes, gd->floor.faces);
  assert_no_glerr();
  glClearColor(0.0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
  draw_model(ui->shader1, ui->floor_model);
  assert_no_glerr();
  glfwSwapBuffers(ui->window);  
}

void game_ui_clear(game_ui * ui){
  glfwMakeContextCurrent(ui->window);
  glClearColor(0.0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void game_ui_swap(game_ui * ui){
  glfwSwapBuffers(ui->window);
  glfwPollEvents();
}

void game_ui_draw_image(game_ui * ui, void *data, int width, int height){
  UNUSED(ui);
  glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
}

void game_ui_draw_angular(game_ui * renderer, float * angle, float * distance, int cnt,
			  float xpos, float ypos, float r,float g, float b, float falloff){
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  load_angle_model(renderer, &renderer->angle_model, angle, distance, cnt);
  angle_model model = renderer->angle_model;
  glUseProgram(renderer->shader2.program);
  glUniform2f(renderer->shader2.offset_uniform, xpos, ypos);
  glUniform2f(renderer->shader2.scale_uniform, 1.0 / 200.0, 1.0 / 200.0);
  glUniform3f(renderer->shader2.color_uniform, r, g, b);
  glUniform1f(renderer->shader2.falloff_uniform, falloff);
  glBindBuffer(GL_ARRAY_BUFFER, model.vertex_buffer);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, 0);
  glVertexAttribPointer(renderer->shader2.vert_attr, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glDrawArrays(GL_TRIANGLE_FAN, 0, model.cnt);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisable(GL_BLEND);
  assert_no_glerr();
}

game_ui * game_ui_init(){
  static bool glfwInited = false;
  if(!glfwInited){
    glfwInit();
    glfwInited = true;
  }
  game_ui r = {0};
  //int * monitor = glfwGetPrimaryMonitor();
  //const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  glfwWindowHint(GLFW_RED_BITS, 12);
  glfwWindowHint(GLFW_GREEN_BITS, 12);
  glfwWindowHint(GLFW_BLUE_BITS, 12);
  r.window = glfwCreateWindow(1000, 1000, "Galaglitch", NULL, NULL);
  r.hashstate = hashstate_new();
  glfwMakeContextCurrent(r.window);
  ASSERT(GLEW_OK == glewInit());
  //glDisable(GL_DITHER);
  load_s1(&r.shader1);
  load_s2(&r.shader2);
  return iron_clone(&r, sizeof(r));
}

void game_ui_deinit(game_ui ** _ui){
  game_ui * ui = *_ui;
  *_ui = NULL;
  
  glfwDestroyWindow(ui->window);
  hashstate_free(&ui->hashstate);
  memset(ui, 0, sizeof(*ui));
}

void game_ui_get_cursor_pos(game_ui * renderer, double * xpos, double * ypos){
  glfwGetCursorPos(renderer->window, xpos, ypos);
}

controller game_ui_get_controller(game_ui * ui){
  int a = glfwGetKey(ui->window, GLFW_KEY_A);
  int d = glfwGetKey(ui->window, GLFW_KEY_D);
  int w = glfwGetKey(ui->window, GLFW_KEY_W);
  int s = glfwGetKey(ui->window, GLFW_KEY_S);
  int esc = glfwGetKey(ui->window, GLFW_KEY_ESCAPE);
  int space = glfwGetKey(ui->window, GLFW_KEY_SPACE);
  controller ctrl = {0};
  if(a)
    ctrl.turn_ratio += 1.0;
  if(d)
    ctrl.turn_ratio -= 1.0;
  if(esc)
    ctrl.exit_clicked = true;
  if(space)
    ctrl.shoot = true;
  if(w)
    ctrl.forward += 1.0;
  if(s)
    ctrl.forward -= 1.0;
  return ctrl;
}


void test_compute_shader(game_ui * ui){
  glfwMakeContextCurrent(ui->window);
  u32 prog = glCreateProgram();
  u32 shader = glCreateShader(GL_COMPUTE_SHADER);
  const char * src = read_file_to_string("resources/c1.compute");
  glShaderSource(shader, 1, &src, 0);
  glCompileShader(shader);
  int r_compile = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &r_compile);
  if(!r_compile){
    
    char log[10240];
    i32 size = 0;
    glGetShaderInfoLog(shader, sizeof(log) -1, &size, log);
    logd("%s", log);
    ERROR("Error compiling compute shader\n");
  }
  glAttachShader(prog, shader);
  glLinkProgram(prog);
  glUseProgram(prog);
  glUniform1i(glGetUniformLocation(prog, "destTex"), 0);


  int pointcnt = 1024 * 8; //* 8;
  u32 vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 2 * (1 + pointcnt) * sizeof(float), NULL, GL_STREAM_DRAW);

  float * buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
  buffer[0] = 0;
  buffer[1] = 0;
  glUnmapBuffer(GL_ARRAY_BUFFER);
  
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, vbo);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vbo);
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, 0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  vec2 p = vec2mk(0,500);
  int power = 100;
  float dir = 0.0;
  float speed = 0.0;
  float t = 0.0;
  while(true){
    u64 t1 = timestamp();
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(prog);
    glUniform1i(glGetUniformLocation(prog, "points"), pointcnt);
    glUniform1f(glGetUniformLocation(prog, "t"), t++);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    controller ctrl = game_ui_get_controller(ui);
    //logd("Ctrl: %f %f\n", ctrl.turn_ratio
    dir -= ctrl.turn_ratio * 0.02;
    vec2 dirvec = vec2mk(sin(dir), cos(dir));

    p = vec2_add(p, vec2_scale(dirvec, speed));
    speed += ctrl.forward;
    

    glUseProgram(prog);
    glUniform2f(glGetUniformLocation(prog, "player_pos"), p.x, p.y);

    if(ctrl.shoot){
      power -= 1;
    }else{
      power += 5;
    }
    
    
    for(int i = -2; i < 10; i++){

      int item = p.y / 250;
      int itemid = (item + i);;
      float x = (itemid % 2) * 500;
      float y = (itemid / 2) * 500;
      if((itemid % 31) == 0 || (itemid % 23 == 0))
	continue;
      if(itemid % 51 == 0 || (itemid + 1) % 51 == 0)
	continue;
      if((itemid + 2) % 51 == 0)
	continue;
      if((itemid + 3) % 51 == 0)
	continue;      
      glUseProgram(prog);
      glUniform2f(glGetUniformLocation(prog, "pos"), x, y);
      
      glDispatchCompute(pointcnt/16, 1, 1);
      glUseProgram(ui->shader2.program);
      glUniform2f(ui->shader2.offset_uniform,  x, y - p.y - 400);
      glUniform2f(ui->shader2.scale_uniform, 1.0 / 800.0, 1.0 / 800.0);
      if((itemid % 13) == 0)
	glUniform3f(ui->shader2.color_uniform, 0.6, 0.3, 0.2);
      else if((itemid % 5) == 0)
	glUniform3f(ui->shader2.color_uniform, 0.5, 0.5, 0.9);
      else
	glUniform3f(ui->shader2.color_uniform, 0.5, 0.7, 0.6);
      glUniform1f(ui->shader2.falloff_uniform, 0.005);
      glDrawArrays(GL_TRIANGLE_FAN, 0, pointcnt + 1);
    }
    float light_power = 0.1;
    if(ctrl.shoot && power > 0){
      light_power = 1.0;
    }
    glUseProgram(prog);
    glUniform2f(glGetUniformLocation(prog, "pos"), p.x + dirvec.x * 21, p.y + dirvec.y * 21);
    
    glDispatchCompute(pointcnt/16, 1, 1);
    glUseProgram(ui->shader2.program);
    glUniform2f(ui->shader2.offset_uniform,  p.x + dirvec.x * 21,  dirvec.y * 21 - 400);
      
    glUniform2f(ui->shader2.scale_uniform, 1.0 / 800.0, 1.0 / 800.0);
    if(power < 20){
      float rest = power * 0.05;
      glUniform3f(ui->shader2.color_uniform,light_power * 1 * sqrtf(rest), light_power * 1 * rest, light_power * 0.8 * rest);
    }else{
	glUniform3f(ui->shader2.color_uniform, light_power * 1, light_power * 1, light_power * 0.8);
      }
      
    glUniform1f(ui->shader2.falloff_uniform, 0.005);
      glDrawArrays(GL_TRIANGLE_FAN, 0, pointcnt + 1);
    
    
    glfwSwapBuffers(ui->window);
    glfwPollEvents();
    u64 t2 = timestamp();
    u64 dt = t2 - t1;\
    if((((int)p.y)%100) == 0)
      logd("dt: %f\n", (float)dt * 0.001);
    iron_usleep(10000);
  }
}
