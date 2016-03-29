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

#include "image.h"
#include "image_filters.h"

i16 * simple_optical_flow(rgb_image * img1, rgb_image * img2, int window_size){
  int window_half = window_size / 2;
  i16 * rel_assignment_buffer = alloc0(img1->width * img1->height * sizeof(i16));
  i16 * assignment_buffer = alloc0(img1->width * img1->height * sizeof(i16));
  float * error_buffer = alloc0(img1->width * img1->height * sizeof(float));
  float * self_error_buffer = alloc0(img1->width * img1->height * sizeof(float));
  for(int i = 0; i < img1->width * img1->height; i++){
    t_rgb px1 = img1->pixels[i];
    t_rgb px2 = img2->pixels[i];
    error_buffer[i] = rgb_error(px1, px2);
    self_error_buffer[i] = error_buffer[i];
    rel_assignment_buffer[i] = window_half + window_half * window_size;
    assignment_buffer[i] = window_half + window_half * window_size;
    //logd("pre err: %f \n", error_buffer[i] );
  }
  
  bool is_done = false;
  while(is_done == false){
    is_done = true;
    for(int j = 0; j < img1->height; j++){
      for(int i = 0; i < img1->width; i++){
	int img1_idx = i + j * img1->width;
	t_rgb px1 = img1->pixels[img1_idx];
	for(int j2 = 0; j2 < window_size; j2++){
	  int jj = (j + j2 - window_half);
	  if(jj < 0 || jj >= img1->height)
	    continue;
	  for(int i2 = 0; i2 < window_size; i2++){
	    int ii = i + i2 - window_half;
	    if(ii < 0 || ii >= img1->width)
	      continue;
	    int img2_idx = ii + jj * img1->width;
	    t_rgb px2 = img2->pixels[img2_idx];
	    float err = rgb_error(px1, px2);
	    //logd("err: %f %f %f\n", err, error_buffer[img2_idx], self_error_buffer[img1_idx]);
	    if(err < error_buffer[img2_idx] && err < self_error_buffer[img1_idx]){
	      //logd("improving..\n");
	      self_error_buffer[img1_idx] = err;
	      assignment_buffer[img1_idx] = i2 + j2 * window_size;
	      i16 prev_assign = rel_assignment_buffer[img2_idx];
	      if(prev_assign != -1){
		is_done = false;
		int i3 = prev_assign % window_size - window_half;
		int j3 = prev_assign / window_size - window_half;
		int prev_idx = (ii + i3) + (jj + j3)* img1->width;
		ASSERT(prev_idx >= 0 && prev_idx < img1->width*img1->height);
		assignment_buffer[prev_idx] = -1;
		self_error_buffer[prev_idx] = 20000;
	      }
	      error_buffer[img2_idx] = err;

	      rel_assignment_buffer[img2_idx] = (window_size - i2 - 1) + (window_size - j2 - 1) * window_size;
	    }
	  }
	}
      }
    }
    break;
  }
  return rel_assignment_buffer;
}

void optical_flow_2(const rgb_image * img1, const rgb_image * img2,
		    vec_image * pred, const int window_size){
  const int w = img1->width, h = img1->height;
  ASSERT(img2->width == w && img2->height == h);
  ASSERT(pred->width == w && pred->height == h);
  ASSERT(window_size % 2 == 1); // window size must be odd.
  const int window_half = window_size / 2;
  vec_image * pred_median = vec_image_new(pred->width, pred->height);
  vec_image * pred_avg = vec_image_new(pred->width, pred->height);

  //median_sample(pred, pred_median);
  average_sample(pred, pred_avg, 3);
  //average_sample(pred_avg, pred_avg);
  //vec_image_print(pred);
  for(int j = 0; j < h; j++){
    for(int i = 0; i < w; i++){
      int img1_idx = i + j * w;
      t_rgb px1 = img1->pixels[img1_idx];
      float error = 1000000;//rgb_error(px1, img2->pixels[img1_idx]);

      vec2 current = vec2_new(0,0);      
      if(false){
	vec2 predv = pred_median->vectors[img1_idx];
	const int jjstart = MAX(0, j - window_half + predv.y);
	const int jjend = MIN(h, j + 1 + window_half + predv.y);
	const int iistart = MAX(0, i - window_half + predv.x);
	const int iiend = MIN(w, i + 1 + window_half + predv.x);
	for(int jj = jjstart; jj < jjend; jj++){
	  for(int ii = iistart; ii < iiend; ii++){
	    vec2 offset = vec2_new(ii - i, jj - j);
	    float penalty = vec2_sqlen(vec2_sub(offset, predv)) * 1.0;
	    int img2_idx = ii + jj * w;
	    t_rgb px2 = img2->pixels[img2_idx];
	    float err = rgb_error(px1, px2) + penalty;
	    if(err < error){
	      error = err;
	      current = offset;//vec2_new(ii - i, jj - j);
	    }
	  }
	}
      }
      
      if(true){
	vec2 predv = pred_avg->vectors[img1_idx];
	const int jjstart = MAX(0, j - window_half + predv.y);
	const int jjend = MIN(h, j + 1 + window_half + predv.y);
	const int iistart = MAX(0, i - window_half + predv.x);
	const int iiend = MIN(w, i + 1 + window_half + predv.x);
	for(int jj = jjstart; jj < jjend; jj++){
	  for(int ii = iistart; ii < iiend; ii++){
	    vec2 offset = vec2_new(ii - i, jj - j);
	    float penalty = vec2_len(vec2_sub(offset, predv));
	    int img2_idx = ii + jj * w;
	    t_rgb px2 = img2->pixels[img2_idx];
	    float err = rgb_error(px1, px2) + penalty;
	    if(err < error){
	      error = err;
	      current = offset;//vec2_new(ii - i, jj - j);
	    }
	  }
	}
      }
      if(false){
	vec2 predv = pred->vectors[img1_idx];
	const int jjstart = MAX(0, j - window_half + predv.y);
	const int jjend = MIN(h, j + 1 + window_half + predv.y);
	const int iistart = MAX(0, i - window_half + predv.x);
	const int iiend = MIN(w, i + 1 + window_half + predv.x);
	for(int jj = jjstart; jj < jjend; jj++){
	  for(int ii = iistart; ii < iiend; ii++){
	    vec2 offset = vec2_new(ii - i, jj - j);
	    float penalty = vec2_sqlen(vec2_sub(offset, predv));
	    int img2_idx = ii + jj * w;
	    t_rgb px2 = img2->pixels[img2_idx];
	    float err = rgb_error(px1, px2) + penalty;
	    if(err < error){
	      error = err;
	      current = offset;//vec2_new(ii - i, jj - j);
	    }
	  }
	}
      }

      
      if(j == 15 && i == 45){
	vec2_print(current);logd("Error : %f\n", error);
      }
      pred->vectors[img1_idx] = current;
    }
  }
  vec_image_delete(&pred_median);
  vec_image_delete(&pred_avg);
}


void compress_scalespace(vec_image ** scalespace, int max_scale){
  for(int i = max_scale - 1; i >= 0; i--){

    const vec_image * v = scalespace[i];
    const vec_image * v2 = scalespace[i + 1];
    const int w = v->width, h = v->height;
    const int w2 = v2->width, h2 = v2->height;
    const float sw = w2 / (float)w, sh = h2 / (float)h;
    vec_image * avgimg = vec_image_new(w,h);
    average_sample(v2, avgimg, 3);
    vec2 scale_add(vec2 a, vec2 avg){
      return vec2_add(a, vec2_div(avg, vec2_new(sw,sh)));
    }

    vec_image_apply2(v, avgimg, (vec_image *)  v, scale_add);
    
    for(int jj = 0; jj < h2; jj++){
      for(int ii = 0; ii < w2; ii++){
	vec2 avg = *vec_image_at(avgimg, ii /sw, jj / sh);
	vec2 * v = vec_image_at((vec_image *) v2, ii, jj);
	*v = vec2_sub(*v, avg);
      }
    }
    vec_image_delete(&avgimg);
  }
}

vec2 calc_scalespace_vector(vec_image ** scalespace, int x, int y, int scale){
  vec2 vector = vec2_new(0,0);
  for(int s = 0; s <= scale; s++){
    int _i = x >> (scale - s);
    int _j = y >> (scale - s);
    int _w = scalespace[scale]->width >> (scale - s);
    int scaleup = 1 << (scale - s);
    vector = vec2_add(vector, vec2_scale(scalespace[s]->vectors[_i + _j * _w], scaleup));
  }
  return vector;
}

#define CLAMP(min, max, value, type) ({type tmp = value; tmp < min ? min : (tmp > max ? max : tmp);})

void optical_flow_3(const rgb_image * img1, const rgb_image * img2,
		    vec_image ** pred_scalespace,
		    const int scale,
		    const int window_size){
  const int w = img1->width, h = img1->height;
  vec_image * pred = pred_scalespace[scale];
  ASSERT(img2->width == w && img2->height == h);
  ASSERT(pred->width == w && pred->height == h);
  ASSERT(window_size % 2 == 1);
  const int window_half = window_size / 2;
  for(int j = 0; j < h; j++){
    for(int i = 0; i < w; i++){
      //int img1_idx = i + j * w;
      t_rgb px1 = *rgb_image_at((rgb_image *)img1, i, j);
      float error = 1000000;
      vec2 current = vec2_new(0, 0);
      vec2 pred2 = calc_scalespace_vector(pred_scalespace, i, j, scale);
      {
	int _i = CLAMP(0, w-1, pred2.x + i, int);
	int _j = CLAMP(0, h-1, pred2.y + j, int);
	
	error = rgb_error(px1, *rgb_image_at((rgb_image *)img2, _i,  _j));
	current = pred2;
      }
      bool improved = false;
      //logd(" %i %i %i\n", i, j, scale);
      
      for(int s = 0; s <= scale; s++){
	if(improved)
	  break;
	int upscale = 1 << (scale - s);
	//logd("s: %i %i\n", s, upscale);
	int _i = i >> (scale - s);
	int _j = j >> (scale - s);
	vec2 predv = calc_scalespace_vector(pred_scalespace, _i, _j, s);
	predv = vec2_scale(predv, upscale);
	/*if(i == 5 && j == 5){
	  logd("Upscale: %i\n", upscale);vec2_print(pred2);vec2_print(predv);logd("\n");
	  }*/

	window_function window = window_function_new(i - window_half + predv.x,
						     j - window_half + predv.y,
						     window_size, window_size,
						     w, h);
	int ii, jj;
	while(window_function_next(&window, &ii, &jj)){
	  ASSERT( ii < w && jj < h);
	  vec2 offset = vec2_new(ii - i, jj - j);
	  float penalty = vec2_len(vec2_sub(offset, pred2));
	  //int img2_idx = ii + jj * w;
	  t_rgb px2 = *rgb_image_at((rgb_image *)img2, ii, jj);//->pixels[img2_idx];
	  float err = rgb_error(px1, px2) + penalty + s;
	  
	  if(err < error){
	    error = err;
	    current = offset;
	    improved = true;
	  }
	}
      }
      if(improved)
	*vec_image_at(pred, i, j) = vec2_add(*vec_image_at(pred, i, j), vec2_sub(current, pred2));
    }
  }
}
