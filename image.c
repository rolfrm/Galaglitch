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
#include <iron/fileio.h>
#include <stdio.h>
#include <stddef.h>

#include "image.h"
void * stbi_load(const char * path, int * width, int * height, int * components, int * req_components);

#include <png.h>
void write_png_file(const char *filename, const int width, const int height, const u8 * data) {

  FILE *fp = fopen(filename, "wb");
  if(!fp)
    ERROR("Could not open '%s'", filename);

  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) ERROR("Could not write PNG version string");

  png_infop info = png_create_info_struct(png);
  if (!info) abort();

  if (setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);
  u8 * row_pointers[height];
  for(int i = 0; i < height; i++){
    row_pointers[i] = (u8 *) (data + i * width * 3);
  }
  // Output is 8bit depth, RGBA format.
  png_set_IHDR(
    png,
    info,
    width, height,
    8,
    PNG_COLOR_TYPE_RGB,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  png_write_info(png, info);

  // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
  // Use png_set_filler().
  //png_set_filler(png, 0, PNG_FILLER_AFTER);

  png_write_image(png, row_pointers);
  png_write_end(png, NULL);
  fclose(fp);
}

t_rgb t_rgb_new(u8 r, u8 g, u8 b){
  t_rgb rgb = {r, g, b};
  return rgb;
}

t_rgb interp(t_rgb p1, t_rgb p2, float interp){
  float iinterp = (1.0 - interp);
  float r = p1.r * interp + p2.r * iinterp;
  float g = p1.g * interp + p2.g * iinterp;
  float b = p1.b * interp + p2.b * iinterp;
  return t_rgb_new(r, g, b);
}

float rgb_error(t_rgb px1, t_rgb px2){
  float  r = px1.r - px2.r;
  float g = px1.g - px2.g;
  float b = px1.b - px2.b;
  return r*r + g*g + b*b;
}

rgb_image * rgb_image_new(int width, int height){

  rgb_image im;
  im.width = width;
  im.height = height;
  im.pixels = alloc0(width * height * sizeof(t_rgb));
  return iron_clone(&im, sizeof(im));
}

void rgb_image_delete(rgb_image ** img_loc){
  rgb_image * img = *img_loc;
  *img_loc = NULL;
  dealloc(img->pixels);
  img->pixels = NULL;
  dealloc(img);
}

rgb_image * load_image(const char * path){
  rgb_image im;
  int fmt = 3;
  im.pixels = stbi_load(path, &im.width, &im.height, &fmt, 0);
  ASSERT(im.pixels != NULL);
  ASSERT(fmt == 3);
  return iron_clone(&im, sizeof(rgb_image));
}

void rgb_image_save(const char * path, const rgb_image * img){
  write_png_file(path, img->width, img->height, (u8 *) &img->pixels);
}

vec_image * vec_image_new(int width, int height){
  vec_image * out = alloc0(sizeof(vec_image) + width * height * sizeof(vec2));
  out->width = width;
  out->height = height;
  out->vectors = ((void *)out) + sizeof(vec_image);
  return out;
}

void vec_image_delete(vec_image ** v){
  dealloc(*v);
  *v = NULL;
}

vec2 * vec_image_at(vec_image * img, int x, int y){
  ASSERT(x >= 0 && x < img->width && y >= 0 && y < img->height);
  return img->vectors + (x + y * img->width);
}

bool vec2_cmp(vec2 v1, vec2 v2){
  return v1.x == v2.x && v1.y == v2.y;
}


void vec_image_print(const vec_image * v){
  for(int j = 0; j < v->height;j++){
    for(int i = 0; i < v->width;i++){
      vec2_print(v->vectors[i + j * v->width]); logd("  ");
    }
    logd("\n");
  }
}
