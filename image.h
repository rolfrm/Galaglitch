// Requires iron/types

bool vec2_cmp(vec2 v1, vec2 v2);
bool vec2_cmpe(vec2 v1, vec2 v2, float epsilon);

typedef struct{
  u8 r,g,b;
}t_rgb;

t_rgb t_rgb_new(u8 r, u8 g, u8 b);
t_rgb interp(t_rgb p1, t_rgb p2, float interp);
float rgb_error(t_rgb px1, t_rgb px2);

// Vec2 Image

typedef struct{
  vec2 * vectors;
  int width, height;
}vec_image;

bool vec2_cmp(vec2 v1, vec2 v2);
vec2 * vec_image_at(vec_image * img, int x, int y);
vec2 vec_image_get(const vec_image * img, int x, int y);
void vec_image_delete(vec_image ** v);
vec_image * vec_image_new(int width, int height);
vec_image * vec_image_new(int width, int height);
void vec_image_delete(vec_image ** v);
void vec_image_print(const vec_image * v);

// RGB Image

typedef struct{
  t_rgb * pixels;
  int height, width;
}rgb_image;

rgb_image * rgb_image_new(int width, int height);
rgb_image * rgb_image_clone(rgb_image * base);
void rgb_image_delete(rgb_image ** img_loc);
rgb_image * load_image(const char * path);
void rgb_image_save(const char * path, const rgb_image * img);
t_rgb * rgb_image_at(rgb_image * img, int x, int y);
t_rgb rgb_image_get(const rgb_image * img, int x, int y);
void rgb_image_print(const rgb_image * img);

// Float Image
typedef struct{
  float * pixels;
  int height, width;
}float_image;

float_image * float_image_new(int width, int height);
void float_image_delete(float_image ** img);
float * float_image_at(float_image * img, int x, int y);
float float_image_get(const float_image * img, int x, int y);
void float_image_print(const float_image * v);
void float_image_save(const char * path, const float_image * v);
void float_image_normalize(float_image * img);
// Window Function
typedef struct{
  int xstart, ystart, xend, yend;
  int _x, _y;
}window_function;

window_function window_function_new(int x, int y, int width, int height, int max_width, int max_height);
bool window_function_next(window_function * w, int * x, int * y);
void window_function_size(window_function * w, int * width, int * height);
int window_function_count(window_function * w);

#define CLAMP(min, max, value, type) ({type tmp = value; tmp < min ? min : (tmp > max ? max : tmp);})
