// requires iron/types
bool vec2_cmp(vec2 v1, vec2 v2);

typedef struct{
  u8 r,g,b;
}t_rgb;

t_rgb t_rgb_new(u8 r, u8 g, u8 b);
t_rgb interp(t_rgb p1, t_rgb p2, float interp);
float rgb_error(t_rgb px1, t_rgb px2);


typedef struct{
  vec2 * vectors;
  int width, height;
}vec_image;

bool vec2_cmp(vec2 v1, vec2 v2);
vec2 * vec_image_at(vec_image * img, int x, int y);
void vec_image_delete(vec_image ** v);
vec_image * vec_image_new(int width, int height);

typedef struct{
  t_rgb * pixels;
  int height, width;
}rgb_image;

rgb_image * rgb_image_new(int width, int height);
void rgb_image_delete(rgb_image ** img_loc);
rgb_image * load_image(const char * path);
void rgb_image_save(const char * path, const rgb_image * img);
vec_image * vec_image_new(int width, int height);
void vec_image_delete(vec_image ** v);
vec2 * vec_image_at(vec_image * img, int x, int y);

void vec_image_print(const vec_image * v);
