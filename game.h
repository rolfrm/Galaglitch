
typedef struct { 
  int * type;
  size_t * vertex;
  float * x;
  float * y;
  float * dx;
  float * dy;
  float * a;
  size_t * model;
  size_t cnt;

}game_entities;

typedef struct{
  float * x;
  float * y;
  float * z;
  int cnt;
}vertex_list;

typedef struct{
  int * f1;
  int * f2;
  int cnt;
}edge_list;

typedef struct{
  int * v1;
  int * v2;
  int * v3;
  int cnt;
}face_list;


typedef struct{
  // Verts for all models. Verts are relative to position.
  vertex_list verts;
  face_list faces;
  
  // A model is defined as a list of edge indexes.
  size_t * start;
  size_t * vert_cnt;
  size_t * cnt;

  // model edges.
  size_t * model_edge;
  size_t model_edge_cnt;
}game_models;

typedef struct{
  vertex_list vertexes;
  face_list faces;
}game_floor;

typedef struct{
  float turn_ratio;
  bool shoot;
  bool exit_clicked;
}controller;

typedef struct{
  game_floor floor;
  game_entities entities;
  game_models models;
}game_data;

controller get_controller();
void render_game(game_entities * e);
void game_iteration(controller controller, float dt, game_data * gd);
void ai_update(const game_data * gd, float dt);
void physics_update(game_data * gd, float dt);
void player_update(const game_data * gd, controller controller, float dt);

struct _game_ui;
typedef struct _game_ui game_ui;

void game_ui_update(game_ui * renderer, const game_data * gd);
game_ui * game_ui_init();
void game_ui_deinit(game_ui ** renderer);
void game_ui_draw_image(game_ui * rnd, void *data, int width, int height);
void game_ui_get_cursor_pos(game_ui * renderer, double * xpos, double * ypos);
void game_ui_clear(game_ui *);
void game_ui_draw_angular(game_ui * renderer, double * angle, double * distance, int cnt,
			  float xpos, float ypos, float r, float g, float b, float falloff);
void game_ui_swap(game_ui *);
controller game_ui_get_controller(game_ui * ui);
