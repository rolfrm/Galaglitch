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

void vec_image_apply_kernel2(const vec_image * img_in, vec_image * img_out,
			    int kernel_width, int kernel_height,
			     vec2 (* f)(vec2 * samples, int x, int y, int w, int h)){
  ASSERT(kernel_width % 2 == 1 && kernel_height % 2 == 1);
  vec2 buffer[kernel_width * kernel_height];
  const int w = img_out->width, h = img_out->height;
  const int w_in = img_in->width, h_in = img_in->height;
  const float wscale = (float)w / (float) w_in;
  const float hscale = (float)h / (float) h_in;

  const int woffset = kernel_width / 2, hoffset = kernel_height / 2;
  
  for(int j = 0; j < h; j++){
    for(int i =0; i < w; i++){
      const int jjstart1 = ceil(j / hscale) - hoffset;
      const int jjstart = MAX(0, jjstart1);
      const int jjend = MIN(h_in,(ceil(j / hscale) + hoffset) + 1);
      const int iistart1 = ceil(i / wscale) - woffset;
      const int iistart = MAX(0, iistart1);
      const int iiend = MIN(w_in,ceil(i / wscale) + woffset + 1);
      int idx = 0;
      memset(buffer, 0, sizeof(buffer));
      for(int jj = jjstart; jj < jjend; jj++){
	for(int ii = iistart; ii < iiend; ii++){
	  buffer[idx++] = img_in->vectors[ii + jj * w_in];
	}
      }
      
      img_out->vectors[i + j * w] = f(buffer, iistart - iistart1, jjstart - jjstart1, iiend - iistart, jjend- jjstart);
    }
  }
}

void vec_image_apply_kernel(const vec_image * img_in, vec_image * img_out,
			    int kernel_width, int kernel_height,
			    vec2 (* f)(int item_cnt, vec2 * samples)){
  vec2 buffer[kernel_width * kernel_height];
  const int w = img_out->width, h = img_out->height;
  const int w_in = img_in->width, h_in = img_in->height;
  const float wscale = (float)w / (float) w_in;
  const float hscale = (float)h / (float) h_in;

  const int woffset = kernel_width / 2, hoffset = kernel_height / 2;
  
  for(int j = 0; j < h; j++){
    for(int i =0; i < w; i++){
      const int jjstart1 = ceil(j / hscale) - hoffset;
      const int jjstart = MAX(0, jjstart1);
      const int jjend = MIN(h_in,(ceil(j / hscale) + hoffset) + 1);
      const int iistart1 = ceil(i / wscale) - woffset;
      const int iistart = MAX(0, iistart1);
      const int iiend = MIN(w_in,ceil(i / wscale) + woffset + 1);
      const int cnt = (iiend - iistart) * (jjend - jjstart);
      int idx = 0;
      memset(buffer, 0, sizeof(buffer));
      for(int jj = jjstart; jj < jjend; jj++){
	for(int ii = iistart; ii < iiend; ii++){
	  buffer[idx++] = img_in->vectors[ii + jj * w_in];
	}
      }
      
      img_out->vectors[i + j * w] = f(cnt, buffer);
    }
  }
}

void vec_image_apply2(const vec_image * img1, const vec_image * img2, vec_image * img_out,
		      vec2 (* f)(vec2 v1, vec2 v2)){
  ASSERT(img1->width == img2->width);
  ASSERT(img1->height == img2->height);
  ASSERT(img1->width == img_out->width);
  ASSERT(img1->height == img_out->height);
  const int cnt = img1->width * img1->height;
  for(int i = 0; i < cnt; i++)
    img_out->vectors[i] = f(img1->vectors[i], img2->vectors[i]);
}

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

static vec2 average_vec_buffer(int cnt, vec2 * samples){
  vec2 avg = vec2_new(0, 0);
  for(int i = 0; i < cnt; i++)
    avg = vec2_add(avg, samples[i]);
  return vec2_scale(avg, 1.0f / cnt);
}

void average_sample(const vec_image * vec_in, vec_image * vec_out, int window_size){
  vec_image_apply_kernel(vec_in, vec_out, window_size, window_size, average_vec_buffer);
}

static vec2 median_vec_buffer(int cnt, vec2 * samples){
  vec2 matches[cnt];
  u8 hits[cnt];
  int match_cnt = 0;
  int max_hits = 0;
  for(int i = 0; i < cnt; i++){
    int j;
    for(j = 0; j < match_cnt; j++){
      if(vec2_cmp(samples[i], matches[j]))
	break;
    }
    if(j == match_cnt){
      matches[match_cnt] = samples[i];
      hits[match_cnt] = 1;
      match_cnt += 1;
    }else{
      hits[j] += 1;
    }
    if(hits[j] > hits[max_hits])
      max_hits = j;
  }
  return matches[max_hits];
}

void median_sample(const vec_image * vec_in, vec_image * vec_out, int window_size){
  vec_image_apply_kernel(vec_in, vec_out, window_size, window_size, median_vec_buffer);
}

static float gaussian(float d, float sigma){
  float sqrootpi_2 = 3.54490770181;
  return expf(-d * d / (2 * sigma * sigma)) *  1.0f / (sigma * sqrootpi_2); 
}

void vec_image_gauss(const vec_image * in, vec_image * out, int window_size, float sigma){
  float gauss[window_size * window_size];
  float whalf = window_size / 2.0f;
  float sum = 0;
  for(int j = 0; j < window_size; j++){
    for(int i = 0; i < window_size; i++){
      float x = 0.5 + i;
      float y = 0.5 + j;
      x = (x - whalf);
      y = (y - whalf);
      float d = sqrtf(x * x + y * y);

      gauss[i + j * window_size] = gaussian(d, sigma);
      sum += gauss[i + j * window_size];
    }
  }
  for(int i = 0; i < window_size * window_size; i++)
    gauss[i] /= sum;
  
  vec2 gauss_filter(vec2 * samples, int x, int y, int w, int h){
    int k = 0;
    vec2 weighted_sum = vec2_new(0, 0);
    for(int j = y; j < h; j++){
      for(int i = x; i < w; i++){
	
	float weight = gauss[i + j * window_size];
	vec2 sample = samples[k++];
	weighted_sum = vec2_add(weighted_sum, vec2_scale(sample, weight));
      }
    }
    
    return weighted_sum;
  }
  vec_image_apply_kernel2(in, out, window_size, window_size, gauss_filter);
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

void rgb_image_conv_kernel(const rgb_image * src, rgb_image * dst, const float_image * kernel, bool normalize){
  //ASSERT(src->width == dst->width);
  //ASSERT(src->height == dst->height);
  const int w = dst->width, h = dst->height;
  const int w2 = src->width, h2 = src->height;
  const int kw = kernel->width, kh = kernel->height;
  const int kw_half = kw / 2, kh_half = kh / 2;
  const float sw = w2 / (float)w, sh = h2 / (float)h;
  window_function win1 = window_function_new(0, 0, w, h, w, h);
  int i, j;
  
  while(window_function_next(&win1, &i, &j)){
    int _i = i * sw, _j = j * sh;
    window_function win2 = window_function_new(_i - kw_half, _j - kh_half, kw, kh, w2, h2);
    float sum = 0.0;
    int ki, kj;
    t_rgb * col = rgb_image_at(dst, i, j);
    
    float r = 0, g = 0, b = 0;
    while(window_function_next(&win2, &ki, &kj)){
      float w = float_image_get(kernel, ki - _i + kw_half, kj - _j + kh_half);
      t_rgb rgb = rgb_image_get(src, ki, kj);
      r += rgb.r * w;
      g += rgb.g * w;
      b += rgb.b * w;
      sum += w;
    }
    
    if(normalize){
      r /= sum;
      g /= sum;
      b /= sum;
    }
    
    r = CLAMP(0, 255, r + col->r, float);
    g = CLAMP(0, 255, g + col->g, float);
    b = CLAMP(0, 255, b + col->b, float);
    
    *col = t_rgb_new(r, g, b);
  }
  
}

void float_image_gaussian_kernel(float_image * kernel, float sigma){
  const int w = kernel->width, h = kernel->height;
  for(int j = 0; j < h; j++){
    for(int i = 0; i < w; i++){
      float x = i;
      float y = j;
      x = (x - w / 2);
      y = (y - h / 2);
      float d = sqrtf(x * x + y * y);

      *float_image_at(kernel, i, j) = gaussian(d, sigma);
    }
  }
}
