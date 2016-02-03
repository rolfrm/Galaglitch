#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <iron/types.h>
#include <iron/mem.h>
#include <iron/fileio.h>
#include <iron/log.h>
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
  int vertex_buffer;
  int vertex_cnt;
  int face_buffer;
  int face_cnt;
  u64 hash;
}ui_model;

struct _game_ui{
  GLFWwindow * window;
  hashstate * hashstate;
  ui_model floor_model;
  s1 shader1;
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

static u64 get_model_hash(hashstate * hashstate, vertex_list verts, face_list faces){
  hashstate_reset(hashstate);
  hashstate_update(hashstate, verts.x, sizeof(*verts.x) * verts.cnt);
  hashstate_update(hashstate, verts.y, sizeof(*verts.y) * verts.cnt);
  hashstate_update(hashstate, verts.z, sizeof(*verts.z) * verts.cnt);
  hashstate_update(hashstate, faces.v1, sizeof(*faces.v1) * faces.cnt);
  hashstate_update(hashstate, faces.v2, sizeof(*faces.v2) * faces.cnt);
  hashstate_update(hashstate, faces.v3, sizeof(*faces.v3) * faces.cnt);
  return hashstate_digest(hashstate);
}

// GL context dependent
static void load_model(game_ui * ui, ui_model * model, vertex_list verts, face_list faces){
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
}



static void draw_model(s1 shader, ui_model model){
  ASSERT(model.vertex_cnt > 0);
  //logd("Vertex buffer: %i\n", model.vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, model.vertex_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.face_buffer);
  glEnableClientState(GL_VERTEX_ARRAY);
  assert_no_glerr();
  glVertexPointer(3, GL_FLOAT, 0, 0);
  glVertexAttribPointer(shader.vert_attr, 3, GL_FLOAT, GL_FALSE, 0, 0);
  assert_no_glerr();
  glDrawElements(GL_TRIANGLES, model.face_cnt * 3, GL_UNSIGNED_INT, 0);
  //glDrawArrays(GL_POINTS, 0, model.vertex_cnt);
  assert_no_glerr();
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDisableClientState(GL_VERTEX_ARRAY);  
}



void game_ui_update(game_ui * ui, const game_data * gd){
  glfwMakeContextCurrent(ui->window);
  glUseProgram(ui->shader1.program);
  assert_no_glerr();
  load_model(ui, &ui->floor_model, gd->floor.vertexes, gd->floor.faces);

  assert_no_glerr();
  glClearColor(0.0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
  draw_model(ui->shader1, ui->floor_model);
  assert_no_glerr();

  glfwSwapBuffers(ui->window);  
}

void game_ui_draw_image(game_ui * ui, void *data, int width, int height){
  glfwMakeContextCurrent(ui->window);
  glClearColor(0.0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
  glfwSwapBuffers(ui->window);	       
}

void game_ui_draw_angular(game_ui * renderer, double * angle, double * distance, int cnt,
			  float xpos, float ypos){

}

game_ui * game_ui_init(){
  static bool glfwInited = false;
  if(!glfwInited){

    glfwInit();

    glfwInited = true;
  }
  game_ui r = {0};
  r.window = glfwCreateWindow(400, 400, "Galaglitch", NULL, NULL);
  r.hashstate = hashstate_new();
  glfwMakeContextCurrent(r.window);
  ASSERT(GLEW_OK == glewInit());
  load_s1(&r.shader1);
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
