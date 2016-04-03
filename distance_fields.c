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
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>

#include "image.h"
#include "image_filters.h"
#include "vr_video_test.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "shader_utils.h"

typedef struct{
  vec3 * centers;
  float * radi;
  t_rgb * colors;
  int cnt;
}spheres;

typedef struct{
  vec3 * centers;
  t_rgb * colors;
  int cnt;
}lights;

typedef struct{
  spheres spheres;
  lights lights;
}distance_field_geometry;

typedef struct{
  mat4 projection;
  mat4 view;
  float f;
}camera;

typedef struct{
  rgb_image * img;
  float * depth;
  int * iterations;
}render_target;


float distance_fcn(distance_field_geometry * geom, vec3 pt, int * _item){
  const spheres sph = geom->spheres;
  float d = 10000;
  int item = -1;
  for(int i = 0; i < sph.cnt; i++){
    //vec3 d_vec = vec3_sub(pt, sph.centers[i]);
    //float r = sph.radi[i];
    //d_vec.x = fabs(d_vec.x) - r;
    //d_vec.y = fabs(d_vec.y) - r;
    //d_vec.z = fabs(d_vec.z) - r;
    //float dsquare = MAX(d_vec.x,MAX(d_vec.y,d_vec.z));
    float dsquare = vec3_len(vec3_sub(pt, sph.centers[i])) - sph.radi[i];
    if(dsquare < d){
      d = dsquare;
      item = i;
    }
  }
  if(item != -1)
    *_item = item;
  return d;
}

typedef struct{
  vec3 pos;
  vec3 size;
}cube;

/*float brick_wall_distance(vec3 pt, vec3 dir, void * data, t_rgb * out_color){
  cube * cube = (cube*) data;
  vec3 offset = 
  }*/

float distance_fcn2(distance_field_geometry * geom, vec3 pt, vec3 dir, int * _item){
  UNUSED(dir);
  const spheres sph = geom->spheres;
  float d = 10000;
  int item = -1;
  for(int i = 0; i < sph.cnt; i++){
    vec3 _pt = sph.centers[i];
    
    //vec3 c = vec3_sub(pt, _pt);
    //float d2 = x * dir.x + y * dir.y + z * dir.z;//vec3_mul_inner(c, dir);
    
    //float dsquare = 0;
    //if(d2 > 0)
    //  dsquare = 10000;
    //else
    //dsquare = vec3_len(c) - sph.radi[i];//sqrtf(x * x + y * y + z * z) - sph.radi[i];
    vec3 d_vec = vec3_sub(pt, _pt);//sph.centers[i]);
    float r = sph.radi[i];
    d_vec.x = fabs(d_vec.x) - r;
    d_vec.y = fabs(d_vec.y) - r;
    d_vec.z = fabs(d_vec.z) - r;
    float dsquare = MAX(d_vec.x,MAX(d_vec.y,d_vec.z));
    if(dsquare < d){
      d = dsquare;
      item = i;
    }
  }
  if(item != -1)
    *_item = item;
  return d;
}


void trace_rays(camera cam, render_target * target, distance_field_geometry * geo){
  t_rgb color_fcn(vec3 o, int idx){
    UNUSED(cam);
    UNUSED(idx);
    int yoff = o.y * 10;
    if(yoff % 3 == 0)
      return t_rgb_new(100, 100, 100);
    yoff = yoff / 3;
    int xoff = o.x * 10;
    if((xoff + yoff * 2 ) % 4 == 0)
      return t_rgb_new(100, 100, 100);
    return t_rgb_new(255, 128, 128);

  }
  int w = target->img->width;
  int h = target->img->height;
  float angles[w];
  
  for(int i = 0; i < w; i++){
    float a = (i / (float) w - 0.5f) * 2.0f;
    angles[i] = sin(a);
  }
  float angle_delta = angles[0] - angles[1];
  angle_delta = sqrtf(angle_delta * angle_delta * 2);
  rgb_image * img = target->img;
  int itemid;

  vec3 center = vec3_new(0,5,0);
  //mat4 projview = mat4_mul(cam.projection, cam.view);
  //mat4 iprojview = mat4_invert(projview);
  //vec3 r = mat4_mul_vec3(mat4_invert(mat4_mul(cam.projection, cam.view)), vec3_new(0.1,0.2,-1));
  //vec3 r = mat4_mul_vec3(iprojview, vec3_new(0,1,-1.0));
  //vec3_print(r); logd("\n");
  vec3 up = vec3_normalize(vec3_new(1, 1, 1));
  vec3 dir = vec3_normalize(vec3_new(1,-1,1));
  vec3 left = vec3_mul_cross(up, dir);
  float min_dist = distance_fcn(geo, center, &itemid);

  for(int j = 0; j < h; j++){
    for(int i = 0; i < w;i++){
      float nj = j / (float) h * 2.0f - 1.0f;
      float ni = i / (float) w * 2.0f - 1.0f;
      
      int iterations = 0;
      int idx = i + (h - j - 1) * w;
      float dist_traveled = min_dist;
      vec3 d = dir;//vec3_normalize(vec3_new(angles[i], angles[j], cam.f));
      float a = 1.0;
      float last_dist = 100.0;
      int prev_item = 0;
      vec3 o = vec3_add(center, vec3_add(vec3_scale(up, nj * 10), vec3_scale(left, ni * 10)));
      while(true){
	iterations++;
	int itemid = 0;
	float dist = distance_fcn2(geo, o, d, &itemid);
	float dpx = angle_delta * dist_traveled;
	if(dist > last_dist){
	  
	  if(last_dist < dpx){
	    if(a > last_dist / dpx){
	      img->pixels[idx] = color_fcn(o, prev_item);
	      a = last_dist / dpx;
	    }
	  }
	}
	if(dist <= 0.0001){

	  img->pixels[idx] = interp(color_fcn(o, itemid), img->pixels[idx], a);
	  break;
	}
	if(dist > 1000){
	  if(a < 1.0)
	    img->pixels[idx] = interp(img->pixels[idx], t_rgb_new(0,0,0), 1.0 - a);
	  break;
	}
	last_dist = dist;
	dist_traveled += dist;
	prev_item = itemid;
	o.x += d.x * dist;
	o.y += d.y * dist;
	o.z += d.z * dist;
      }
      target->depth[idx] = dist_traveled;
      target->iterations[idx] = iterations;
    }
  }
}

bool distance_field_test_3k(){
  rgb_image * img = rgb_image_new(512, 512);
  float * depth = alloc0(sizeof(float) * img->width * img->height);
  int * iterations = alloc0(sizeof(int) * img->width * img->height);
  camera cam = {/*mat4_ortho(-10,10,-10,10,0.1,100)*/mat4_perspective(1.0,1.0,0.1,100.0), mat4_look_at(vec3_new(3,0,3), vec3_new(4,0,4), vec3_new(0,2,0)), 1.0};
  render_target target = { img, depth, iterations};

  vec3 sphere_centers[] = {vec3_new(0, 0, 0), vec3_new(6, 0, 2)};
  t_rgb sphere_colors[] = {t_rgb_new(255,0,0), t_rgb_new(0,255,0)};
  float sphere_radi[] = {1.0, 1.0};
    distance_field_geometry dfg = { .spheres = (spheres){.centers = sphere_centers, .colors = sphere_colors, .radi = sphere_radi, .cnt = array_count(sphere_radi)}, .lights = (lights){}};
    //for(float i = 0; i < 3.0; i += 0.2){
    sphere_centers[0].y = 0;//i * 2;
      u64 ts = timestamp();
      trace_rays(cam, &target, &dfg);
      u64 tsend = timestamp();
      logd("DT: %f ms", (tsend - ts) * 0.001f);

      char buf[100];
      sprintf(buf, "distance_field_%f.png", sphere_centers[0].y);
      rgb_image_save(buf, img);
      float depth_min = 10000;
      float depth_max = -10000;
      for(int i = 0; i < img->width * img->height; i++){
	if(depth[i] < 10){
	  if(depth[i] < depth_min)
	    depth_min = depth[i];
	  if(depth[i] > depth_max)
	    depth_max = depth[i];
	}
      }
      depth_min = 0;
      depth_max = 10;
      for(int i = 0; i < img->width * img->height; i++){
	//img->pixels[i] = t_rgb_new(iterations[i],iterations[i],iterations[i]);
	u8 col = (u8) CLAMP(0, 255, (depth[i] - depth_min) * (depth_max - depth_min), float);
	img->pixels[i] = t_rgb_new(col, col, col);
      }
      rgb_image_save("distance_field_it.png", img);
      //}
  rgb_image_delete(&img);

  return TEST_SUCCESS;
}
