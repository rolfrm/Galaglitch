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
#include "image_filters.h"

vec_image * vec_image_scaleup(const vec_image * v, const int new_width, const int new_height){
  vec_image * newimg = vec_image_new(new_width, new_height);
  const float scalex = v->width / (float)new_width;
  const float scaley = v->height / (float)new_height;
  for(int j = 0; j < new_height; j++){
    for(int i = 0; i < new_width; i++){
      float fi = i * scalex;
      float fj = j * scaley;
      int i1 = round(fi);
      int j1 = round(fj);
      i1 = MIN(MAX(0,i1), v->width - 1);
      j1 = MIN(MAX(0,j1), v->height - 1);
      newimg->vectors[i + j * new_width] = vec2_div(v->vectors[i1 + j1 * v->width], vec2_new(scalex, scaley));      
    }
  }
  return newimg;
}

void average_sample(const vec_image * vec_in, vec_image * vec_out){
  const int w = vec_out->width, h = vec_out->height;
  const int w_in = vec_in->width, h_in = vec_in->height;
  const float scale = (float)w / (float) w_in;
  for(int j = 0; j < h; j++){
    for(int i =0; i < w; i++){
      const int jjstart = MAX(0, (j / scale - 1));
      const int jjend = MIN(h_in,(j / scale + 1) + 1);
      const int iistart = MAX(0, (i / scale - 1));
      const int iiend = MIN(w_in,(i / scale) + 1 + 1);
      const int cnt = (iiend - iistart) * (jjend - jjstart);
      vec2 avg = vec2_new(0,0);
      for(int jj = jjstart; jj < jjend; jj++){
	for(int ii = iistart; ii < iiend; ii++){
	  vec2 vec = vec_in->vectors[ii + jj * w_in];
	  avg = vec2_add(avg, vec);
	}
      }
      
      vec_out->vectors[i + j * w] = vec2_scale(avg, 1.0f / cnt);
    }
  }
}

void median_sample(const vec_image * vec_in, vec_image * vec_out){
  const int w = vec_out->width, h = vec_out->height;
  const int w_in = vec_in->width, h_in = vec_in->height;
  const float scale = (float)w / (float) w_in;
  for(int j = 0; j < h; j++){
    for(int i =0; i < w; i++){
      const int jjstart = MAX(0, round(j / scale - 2));
      const int jjend = MIN(h_in, round(j / scale + 2) + 1);
      const int iistart = MAX(0, round(i / scale - 2));
      const int iiend = MIN(w_in, round(i / scale) + 2 + 1);
      const int cnt = (iiend - iistart) * (jjend - jjstart);
      vec2 items[cnt];
      i8 matches[cnt];
      memset(matches,0,sizeof(matches));
      int vec_i = 0;
      for(int jj = jjstart; jj < jjend; jj++){
	for(int ii = iistart; ii < iiend; ii++){
	  vec2 vec = vec_in->vectors[ii + jj * w_in];
	  for(int k = 0; k < vec_i; k++){
	    if(vec2_cmp(vec, items[k])){
	      matches[k]++;
	      goto next_vec;
	    }
	  }

	  matches[vec_i] = 1;
	  items[vec_i++] = vec;
	next_vec:;
	}
      }
      
      int max_idx = 0;
      for(int k = 1; k < vec_i; k++){
	if(matches[k] > matches[max_idx]){
	  max_idx = k;
	}
      }
      vec_out->vectors[i + j * w] = items[max_idx];
    }
  }
}

rgb_image * rgb_image_blur_subsample(rgb_image * img, float scale){
  int width = MAX(1, img->width * scale);
  int height = MAX(1, img->height * scale);
  t_rgb * image = alloc0(width * height * sizeof(t_rgb));
  
  for(int j = 0; j < height; j++){
    for(int i = 0; i < width; i++){
      int iistart = MAX(0,i/scale - 1);
      int jjstart = MAX(0,j/scale - 1);
      int iiend = MIN(img->width, i/scale + 2);
      int jjend = MIN(img->height, j/scale + 2);
      int r = 0;
      int g = 0;
      int b = 0;
      for(int ii = iistart; ii < iiend; ii++)
	for(int jj = jjstart; jj < jjend; jj++){
	  t_rgb px = img->pixels[ii + jj * img->width];
	  r += px.r;
	  g += px.g;
	  b += px.b;
	}
      int cnt = (iiend - iistart) * (jjend - jjstart);
      t_rgb px;
      px.r = r / cnt;
      px.g = g / cnt;
      px.b = b / cnt;
      //if(j == 5){
      // printf("CNT: %i %i %i %i %i\n", cnt, iistart, iiend, jjstart, jjend);
      //}
      image[i + j * width] = px;
    }
  }
  rgb_image imgout;
  imgout.pixels = image;
  imgout.width = width;
  imgout.height = height;
  return iron_clone(&imgout, sizeof(imgout));
}


rgb_image * rgb_image_subsample(rgb_image * img, float scale){
  int width = MAX(1, img->width * scale);
  int height = MAX(1, img->height * scale);
  t_rgb * image = alloc0(width * height * sizeof(t_rgb));
  
  for(int j = 0; j < height; j++){
    float oj = j / scale - scale;
    int j1 = MAX(0, floor(oj));
    int j2 = MAX(0, ceil(oj));
    float j_interp = oj - floor(oj);
    //logd("J interp %f %f %i %i\n", j_interp, oj, j1, j2);      
    for(int i = 0; i < width; i++){
      float oi = ((float) i) / scale - scale;
      float i_interp = oi - floor(oi);
      int i1 = MAX(0, floor(oi));
      int i2 = MAX(0, ceil(oi));

      t_rgb c11 = img->pixels[i1 + j1 * img->width];
      t_rgb c21 = img->pixels[i2 + j1 * img->width];
      t_rgb c12 = img->pixels[i1 + j2 * img->width];
      t_rgb c22 = img->pixels[i2 + j2 * img->width];
      // billinear interpolation.
      image[i + j * width] = interp(interp(c11, c21, i_interp), interp(c12, c22, i_interp), j_interp);
    }
  }
  rgb_image imgout;
  imgout.pixels = image;
  imgout.width = width;
  imgout.height = height;
  return iron_clone(&imgout, sizeof(imgout));
}
