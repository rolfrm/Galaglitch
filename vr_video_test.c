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
#include "optical_flow.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "shader_utils.h"

bool vec_image_median_test(){
  vec_image * v1 = vec_image_new(5,5);
  vec_image * v2 = vec_image_new(3,3);
  for(int j = 0; j < v1->height;j++){
    for(int i = 0; i < v1->width;i++){
      v1->vectors[i + j * v1->width] = vec2_new(i % 2 + j % 2,i % 2);
    }
  }
  median_sample(v1,v2, 3);
  vec_image_print(v1);
  vec_image_print(v2);
  ASSERT(vec2_cmp(vec2_new(2,1), *vec_image_at(v2, 1, 1)));
  ASSERT(vec2_cmp(vec2_new(0,0), *vec_image_at(v2, 0, 0)));

  return TEST_SUCCESS;
}


bool vec_image_average_test(){
  vec_image * v1 = vec_image_new(5,5);
  vec_image * v2 = vec_image_new(3,3);
  *vec_image_at(v1,0,0) = vec2_new(1,1);
  //*vec_image_at(v1,2,2) = vec2_new(2,2);
  *vec_image_at(v1,4,4) = vec2_new(-1,-1);
  average_sample(v1, v2, 3);
  //vec_image_gauss(v1, v2, 3, 1.0);
  vec_image_print(v1);
  vec_image_print(v2);
  //ASSERT(vec2_cmp(vec2_new(2,1), v2->vectors[1 + 1 * 3]));
  ASSERT(vec2_cmp(vec2_new(0.25,0.25), v2->vectors[0]));
  ASSERT(vec2_cmp(vec2_new(-0.25,-0.25), *vec_image_at(v2,2,2)));

  return TEST_SUCCESS;
}



bool optical_flow_single_scale_test(){
  rgb_image * img1 = load_image("img1_6.png");
  rgb_image * img2 = load_image("img2_6.png");
  //rgb_image * img1 = load_image("im.png");
  //rgb_image * img2 = load_image("im2.png");
  vec_image * pred = vec_image_new(img1->width, img1->height);
  for(int i = 0; i < 5; i++){
    optical_flow_2(img1, img2, pred, 9);

    char buf[100];
    sprintf(buf, "testout/%i.png", i);
    vec2 avg = save_pred(pred, buf);
    logd("%i average: ", i);vec2_print(avg);logd("\n");

  }
  vec2_print(pred->vectors[24 + 36 * pred->width]); logd("  %i \n",25 + 34 * pred->width);
  
  //vec_image_print(pred);
  return TEST_SUCCESS;
}

bool optical_flow_test(){

  rgb_image * img1 = load_image("IMG1.jpg");
  rgb_image * img2 = load_image("IMG2.jpg");
  rgb_image * ss1[10];
  rgb_image * ss2[10];
  construct_scalespace(ss1, img1, array_count(ss1));
  construct_scalespace(ss2, img2, array_count(ss2));
  {
    rgb_image * im = ss1[6];
    rgb_image * im2 = ss2[6];
    //im->pixels[0].r = 0xFF;
    //im2->pixels[1].r = 0xFF;
    rgb_image_save("img1_6.png", im);
    rgb_image_save("img2_6.png", im2);
  }
  //return TEST_SUCCESS;
  TEST_ASSERT(img1->width == img2->width);
  TEST_ASSERT(img2->height == img2->height);
  //flow_image * flow = flow_image_new(img1->width, img1->height);
  rgb_image * im = load_image("im21.png");
  rgb_image * im2 = load_image("im22.png");
  logd("of of img: %i x %i\n", im->width, im->height);
  //return TEST_SUCCESS;
  for(int i = 0; i < 1; i++){

    int window = 5;
    u64 ts1 = timestamp();
    i16 * rel = simple_optical_flow(im, im2, window);
    u64 ts2 = timestamp();
    /*for(int j = 0; j < im->height; j++){
      for(int i = 0; i < im->width; i++){
	
	logd("%2i %2i |", rel[i + j * im->width] % window - window / 2, rel[i + j * im->width] / window - window / 2);
      }
      logd("\n");
      }*/
    vec2 total_vec = vec2_new(0,0);
    logd("dt: %i\n", ts2 - ts1);
    rgb_image * vim = rgb_image_new(im->width, im->height);
    for(int j = 0; j < im->height; j++){
      for(int i = 0; i < im->width; i++){
	float x = rel[i + j * im->width] % window - window/2;
	float y = rel[i + j * im->width] / window - window/2;

	x /= window;
	y /= window;
	total_vec = vec2_add(vec2_new(x,y), total_vec);
	t_rgb pseu= {0,0,0};
	if(x < 0){
	  pseu.r = (-x) * 2 * 255;
	}else{
	  pseu.b = x * 2 * 255;
	}
	vim->pixels[i + j * im->width] = pseu;
	
      }
    }
    logd("Total: ");
    vec2_print(vec2_div(total_vec, vec2_new(im->width * im->height, im->width * im->height)));
    logd("\n");
    if(i == 0){
      char buffer[100];
      memset(buffer,0,sizeof(buffer));
      sprintf(buffer, "vim_%i.png", i);
      printf("writing data to png: %s %i %i\n", buffer, vim->width, vim->height);
      rgb_image_save(buffer, vim);
    }
    SWAP(im,im2);
  }
  
  return TEST_SUCCESS;
}



bool scale_vec_test(){
  vec_image * pred = vec_image_new(3, 3);
  pred->vectors[0 + 3] = vec2_new(1,1);
  pred->vectors[0 + 4] = vec2_new(1,-1);
  pred = vec_image_scaleup(pred, 7,5 );
  vec_image_print(pred);
  return TEST_SUCCESS;
}


bool test_visualize_flow(){
  rgb_image * img1 = load_image("scalespace/ss1_6.png");
  rgb_image * img2 = load_image("f1/IMG2.jpg");
  visualize_flow(img1,img2,NULL, NULL);
  return TEST_SUCCESS;
}

/*bool test_scalespace_calc(){
  vec_image * scalespace[] = {vec_image_new(16,16), vec_image_new(32,32)};
  }*/
//vec2 calc_scalespace_vector(vec_image ** scalespace, int x, int y, int scale)
void scalespace_print(vec_image ** scalespace, int scale){
  vec_image * img = scalespace[scale];
  vec_image * img2 = vec_image_new(img->width, img->height);
  for(int j = 0; j < img2->height; j++){
    for(int i = 0; i < img2->width; i++){
      int sig;
      *vec_image_at(img2, i,j) = calc_scalespace_vector(scalespace, i, j, scale, &sig);
    }
  }
  vec_image_print(img2);
  save_pred(img2, "testest.png");
  vec_image_delete(&img2);
  
}

void create_scalespaces(int n);
bool optical_flow_single_scale_test3(){
  //create_scalespaces(4);
  ensure_directory("testout2");
  ensure_directory("results");
  const int sub_space_cnt = 4;
  const int img_scale_cnt = 4;
  char buf[100]; 
  int idx = 0;

  {
    vec_image * scalespace[img_scale_cnt + sub_space_cnt];
    memset(scalespace, 0, sizeof(scalespace));
    {
      sprintf(buf, "scalespace2_1/%i.png", img_scale_cnt - 1);
      rgb_image * img = load_image(buf);
      int w = img->width;
      int h = img->height;
      rgb_image_delete(&img);
      for(int i = img_scale_cnt + sub_space_cnt - 1; i >= 0; i--){
	scalespace[i] = vec_image_new(w,h);
	w /= 2;
	h /= 2;
      }
      for(int i = 0; i < img_scale_cnt + sub_space_cnt;i++){              
	vec_image * s = scalespace[i];
	if(s == NULL){
	  logd("Null..\n");
	} else {
	  logd("lod: %i (%i %i)\n", i,  s->width, s->height);
	}
      }
    }


    for(int img_scale = 0; img_scale < img_scale_cnt ; img_scale++){
      int sub_space = img_scale + sub_space_cnt;
      ASSERT(sub_space >= 0);
      sprintf(buf, "scalespace2_1/%i.png", img_scale);
      rgb_image * img1 = load_image(buf);
      sprintf(buf, "scalespace2_2/%i.png", img_scale);
      rgb_image * img2 = load_image(buf);
      logd("size: %i %i\n", img1->width, img1->height);

      sprintf(buf, "testout2/%i scaleup.png", idx++);
      save_pred(scalespace[sub_space], buf);
      float_image * error = float_image_new(img1->width, img1->height);
      for(int it = 0; it < 5; it++){
	optical_flow_3(img1, img2, scalespace, error, sub_space, 13);

	sprintf(buf, "testout2/%i_error.png", idx);
	float_image_normalize(error);
	float_image_save(buf, error);

	for(int i = 0; i < 10; i++)
	  compress_scalespace(scalespace, sub_space);
	idx++;
      }
      rgb_image * testimg = rgb_image_new(img1->width, img1->height);
      for(float t =0; t <= 1.09; t+= 0.1){

	memset(testimg->pixels,0,testimg->width * testimg->height * sizeof(testimg->pixels[0]));
	for(int y = 0; y < img1->height; y++){
	  for(int x = 0; x < img1->width; x++){
	    if(float_image_get(error, x, y) > 1)
	      continue;
	    int sig;
	    vec2 vector = calc_scalespace_vector(scalespace, x, y, sub_space, &sig);
	    vec2 pt = vec2_add(vec2_new(x,y), vec2_scale(vector, t));
	    t_rgb * px = rgb_image_at(testimg, pt.x, pt.y);
	    if(px != NULL)
	      *px = *rgb_image_at(img1, x, y);
	  }
	}
	sprintf(buf, "results/%i_%f.png", idx,t);
	rgb_image_save(buf, testimg);
      }
    
      
      /*if(i == 5){
	visualize_flow(img1, img2,pred,pred);
	return TEST_SUCCESS;
	}*/
      rgb_image_delete(&img1);
      rgb_image_delete(&img2);
      rgb_image_delete(&testimg);
      float_image_delete(&error);
      //scalespace_print(scalespace, sub_space_cnt);
    }
  }
  return TEST_SUCCESS;
}

bool optical_flow_single_scale_test2(){
  //optical_flow_init();
  char buf[100]; 
  int idx = 0;
  {
    int sizes[15] ={3,3,3,3,5,5,9,9,5,5};
    vec_image * pred = vec_image_new(1, 1);
    for(int i = 8; i >=5 ; i--){

      sprintf(buf, "scalespace/ss1_%i.png", i);
      rgb_image * img1 = load_image(buf);
      sprintf(buf, "scalespace/ss2_%i.png", i);
      rgb_image * img2 = load_image(buf);
      vec_image * npred = vec_image_scaleup(pred, img1->width, img1->height);
      vec_image_delete(&pred);
      pred = npred;


      
      logd("size: %i %i\n", pred->width, pred->height);
      sprintf(buf, "testout2/%i scaleup.png", idx++);
      save_pred(pred, buf);
      int _i = i;
      for(int i = 0; i < 10; i++){
	optical_flow_2(img1, img2, pred, sizes[_i]);

	sprintf(buf, "testout2/%i _img.png", idx++);
	vec2 avg = save_pred(pred, buf);
	vec2_print(avg);logd("\n");
      }

      rgb_image * testimg = rgb_image_new(img1->width, img1->height);
      for(float t =0; t <= 1.09; t+= 0.1){
	memset(testimg->pixels,0,testimg->width * testimg->height * sizeof(testimg->pixels[0]));
	for(int y = 0; y < img1->height; y++){
	  for(int x = 0; x < img1->width; x++){
	    vec2 pt = vec2_add(vec2_new(x,y), vec2_scale(pred->vectors[x + y * img1->width], t));
	    int idx = pt.x + (int) pt.y * img1->width;
	    if(pt.x >= 0 && pt.x < img1->width && pt.y >= 0 && pt.y < img1->height)
	      testimg->pixels[idx] = img1->pixels[x + y * img1->width];
	  }
	}
	sprintf(buf, "results/%i_%f.png", idx,t);
	rgb_image_save(buf, testimg);
      }
      
      /*if(i == 5){
	visualize_flow(img1, img2,pred,pred);
	return TEST_SUCCESS;
	}*/
      rgb_image_delete(&img1);
      rgb_image_delete(&img2);
      rgb_image_delete(&testimg);
      
    }
    vec_image_delete(&pred);
  }
  return TEST_SUCCESS;
}


void optical_flow_init(){
  rgb_image * _img1 = load_image("f1/IMG1.jpg");
  rgb_image * _img2 = load_image("f1/IMG2.jpg");
  rgb_image * ss1[10];
  rgb_image * ss2[10];
  construct_scalespace(ss1, _img1, array_count(ss1));
  construct_scalespace(ss2, _img2, array_count(ss2));
  for(size_t i = 0; i < array_count(ss1); i++){
    char buf[100];
    sprintf(buf, "scalespace/ss1_%i.png", i);
    rgb_image_save(buf, ss1[i]);
    sprintf(buf, "scalespace/ss2_%i.png", i);
    rgb_image_save(buf, ss2[i]);
    rgb_image_delete(ss1 + i);
    rgb_image_delete(ss2 + i);
  }
}


void construct_scalespace(rgb_image ** ss, rgb_image * img, int count){
  ss[0] = img;
  for(int i = 1; i < count; i++){

    img = rgb_image_blur_subsample(img, 0.5);
    ss[i] = img;
  }
}

vec2 save_pred(const vec_image * pred, const char * path){
  vec2 avg = vec2_new(0,0);
  float scale = 1.0 / (pred->width * pred->height);
  rgb_image * out = rgb_image_new(pred->width,pred->height);
  float xmax = 0,ymax = 0;
  for(int i = 0; i < pred->width * pred->height; i++){
    xmax = MAX(xmax, fabs(pred->vectors[i].x));
    ymax = MAX(ymax, fabs(pred->vectors[i].y));
  }
  vec2 max = vec2_new(xmax, ymax);
  for(int i = 0; i < pred->width * pred->height; i++){


    vec2 offset = pred->vectors[i];
    vec2 scaled = vec2_add(vec2_new(0.5,0.5), vec2_scale(vec2_div(offset, max), 0.5));
    scaled = vec2_scale(scaled, 255);
    if(i % pred->width == pred->width / 2){
      //vec2_print(offset);logd("\n");
    }
      out->pixels[i] = (t_rgb){scaled.x, scaled.y, 0};

      avg = vec2_add(avg, offset);//avg, vec2_scale(offset, scale));
    //vec2 init = vec2_new(i % pred->width, i / pred->width);
    //float ol = vec2_len(offset);
    //int _x = i % pred->width;
    //int _y = i / pred->width;    
    //if(_x % 2 || _y % 2) continue;
    /*
      float s = 1 / ol;
      for(float i = 0; i <1.0; i += s){
      vec2 pt = vec2_add(init, vec2_scale(offset, i));
      int x = pt.x;
      int y = pt.y;
      if(x >= 0 && x < out->width && y >= 0 && y< out->height){
	out->pixels[x + y * out->width].r += 10 * (1.0 - i);
	out->pixels[x + y * out->width].g += 10 * (1.0 - i);
	out->pixels[x + y * out->width].b += 10 * (1.0 - i);
      }
      }*/
    //vec2_print(pred->vectors[i]);logd("\n");

  }
  rgb_image_save(path, out);
  return vec2_scale(avg, scale);
}



u32 load_rgb_image_texture(rgb_image * im){

  u32 glref = 0;
  glGenTextures(1, &glref);
  glBindTexture(GL_TEXTURE_2D, glref);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, im->width, im->height, 0, GL_RGB, GL_UNSIGNED_BYTE, im->pixels);
  glBindTexture(GL_TEXTURE_2D, 0);
  return glref;
}
u32 load_vec_image_texture(vec_image * im){
  u32 glref = 0;
  glGenTextures(1, &glref);
  glBindTexture(GL_TEXTURE_2D, glref);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, im->width, im->height, 0, GL_RG, GL_FLOAT, im->vectors);
  glBindTexture(GL_TEXTURE_2D, 0);
  return glref;
}

void visualize_flow(rgb_image * im1, rgb_image * im2, vec_image * v1_2, vec_image * v2_1){
  glfwInit();
  GLFWwindow * win = glfwCreateWindow(200, 200, "Galaglitch", NULL, NULL);
  glfwMakeContextCurrent(win);
  glewInit();
  UNUSED(v1_2);UNUSED(v2_1);
  u32 tex1 = load_rgb_image_texture(im1);
  u32 tex2 = load_rgb_image_texture(im2);
  u32 tex_f1 = load_vec_image_texture(v1_2);
  u32 tex_f2 = load_vec_image_texture(v2_1);
  logd("%i %i %i %i\n", tex1, tex2, tex_f1, tex_f2);

  size_t s1_vert_size, s1_frag_size;
  char * s1_vert = read_file_to_buffer("assets/flow.vert", &s1_vert_size);
  char * s1_frag = read_file_to_buffer("assets/flow.frag", &s1_frag_size);
  int shader = load_simple_shader(s1_vert, s1_vert_size, s1_frag, s1_frag_size);
  glUseProgram(shader);
  logd("Shader: %i\n", shader);
  u32 textures[] = {tex1, tex2, tex_f1, tex_f2};
  const char * locs[] = {"im1", "im2", "f1_2", "f2_1"};
  for(u32 i = 0; i < array_count(textures); i++){
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glUniform1i(glGetUniformLocation(shader, locs[i]), 0);
  }
  while(true){
    glClearColor(1.0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_QUADS, 0, 4);
    glfwSwapBuffers(win);
    iron_usleep(10000);
  }
}

bool compress_scalespace_test(){
  int ws[] = {5, 9, 14, 3, 21, 6, 10,18, 190}; //18 / 9, 10 / 5
  int hs[] = {7, 10, 16, 11, 23, 3, 5,9, 65};
  vec_image * levels[10];
  for(u32 it = 0; it < array_count(ws); it++){
    int cnt = 0;
    int w = ws[it];
    int h = hs[it];
    { // calc scalespace
      float _w = w;
      float _h = h;
      while(_w >= 1.0 && _h >= 1.0){
	levels[cnt++] = vec_image_new(_w, _h);
	_w *= 0.5;
	_h *= 0.5;
	//logd("level: %i %i\n", levels[cnt - 1]->width, levels[cnt - 1]->height);
      }
      vec_image * levels2[cnt];
      for(int i = 0; i < cnt; i++)
	levels2[i] = levels[cnt - i - 1];
      for(int i = 0; i < cnt; i++)
	levels[i] = levels2[i];
    }
    
    vec_image * s_last = levels[cnt - 1];

    int last_idx = cnt - 1;
    {
      for(int j = 0; j < s_last->height; j++){
	for(int i = 0; i < s_last->width; i++){
	  *vec_image_at(s_last,i,j) = vec2_new(i,j);
	}
      }
    }
    
    int scalespace_iterations = 5;
    {
      const char * ssi = getenv("SCALESPACE_ITERATIONS");
      if(ssi != NULL){
	char * endptr = NULL;
	int v = strtol(ssi, &endptr, 10);
	if(endptr != NULL)
	  scalespace_iterations = v;
      }
    }
    

    for(int i = 0; i < scalespace_iterations; i++)
      compress_scalespace(levels, last_idx);
    //vec_image_print(levels[2]);logd("\n");
    //vec_image_print(levels[1]);logd("\n");
    //vec_image_print(levels[0]);logd("\n");
    /*for(int j = 0; j < s_last->height; j++){
      for(int i = 0; i < s_last->width; i++){
	vec2 backcalc = calc_scalespace_vector(levels,i,j,last_idx);
	vec2_print(backcalc);logd("\n");
      }
      logd("\n");
      }*/
    for(int j = 0; j < s_last->height; j++){
      for(int i = 0; i < s_last->width; i++){
	int sig;
	vec2 backcalc = calc_scalespace_vector(levels,i,j,last_idx, &sig);
	//vec2_print(backcalc);logd("\n");
	ASSERT(vec2_cmpe(backcalc, vec2_new(i,j), 0.001));
      }
    }

    for(int i = 0; i < cnt; i++)
      vec_image_delete(&levels[i]);
  }
  
  return TEST_SUCCESS;
}

bool window_function_test() {
  int startx = 5, starty = 0, window_width = 20, window_height = 20;
  window_function wf = window_function_new(startx,starty,window_width,window_height, 9, 8);
  int x, y;
  bool hit_mid = false;
  while(window_function_next(&wf, &x, &y)){
    //logd("x:%i y:%i\n", x, y);
    ASSERT(x < 9);
    ASSERT(y < 8);
    ASSERT(x >= startx);
    ASSERT(y >= starty);
    hit_mid |=  (x == 7 && y == 7);
  }
  ASSERT(window_function_count(&wf) == (9 - startx) * (8 - starty));
  ASSERT(hit_mid);
  return TEST_SUCCESS;
}

void create_scalespace_2(const char * base_img, const char * folder, int n_scales){
  ensure_directory(folder);
  float_image * kernel = float_image_new(7, 7);
  float_image_gaussian_kernel(kernel, 1.3);
  rgb_image * img = load_image(base_img);
  while(true){
    char buf[100];
    sprintf(buf, "%s/%i.png", folder, n_scales);
    logd("saving as %s\n", buf);
    rgb_image_save(buf, img);
    //rgb_image * new = rgb_image_new(base->width / 2, base->height / 2);
    if(n_scales == 0)
      break;
    n_scales--;
    rgb_image * next = rgb_image_new(img->width / 2, img->height / 2);
    rgb_image_conv_kernel(img, next, kernel, true);
    rgb_image_delete(&img);
    img = next;
  }
  rgb_image_delete(&img);
  float_image_delete(&kernel);
}

void create_scalespaces(int cnt){
  //create_scalespace_2("f1/IMG1.jpg", "scalespace2_1", cnt);
  //create_scalespace_2("f1/IMG2.jpg", "scalespace2_2", cnt);
  create_scalespace_2("img1.png", "scalespace2_1", cnt);
  create_scalespace_2("img2.png", "scalespace2_2", cnt);
}

bool image_kernel_test(){
  
  int width = 16, height = 21;
  rgb_image * base = rgb_image_new(width, height);
  rgb_image * kernel_test = rgb_image_new(base->width, base->height);
  float_image * kernel = float_image_new(3, 3);
  
  *rgb_image_at(base, width / 2, height / 2) = t_rgb_new(100, 100, 100);
  *rgb_image_at(base, width / 2 + 1, height / 2) = t_rgb_new(100, 100, 100);
  *rgb_image_at(base, width / 2 -1, height / 2) = t_rgb_new(100, 100, 100);
  for(int j = 0; j < 3; j++){
    for(int i = 0; i < 3; i++){
      *float_image_at(kernel, i, j) = 9;
    }
  }
  
  rgb_image_conv_kernel(base, kernel_test, kernel, true);
  
  { // tests
    t_rgb * cpixs = rgb_image_at(kernel_test, width / 2, height / 2);
    int centerc = 300 / 9, offset = 0;
    TEST_ASSERT(cpixs[offset].r == centerc && cpixs[offset].g == centerc && cpixs[offset].b == centerc);
    centerc = 200 / 9; offset = -1;
    TEST_ASSERT(cpixs[offset].r == centerc && cpixs[offset].g == centerc && cpixs[offset].b == centerc);
    offset = -1;
    TEST_ASSERT(cpixs[offset].r == centerc && cpixs[offset].g == centerc && cpixs[offset].b == centerc);
    offset = -2; centerc = 100 / 9;
    TEST_ASSERT(cpixs[offset].r == centerc && cpixs[offset].g == centerc && cpixs[offset].b == centerc);
    offset = -3; centerc = 0;
    TEST_ASSERT(cpixs[offset].r == centerc && cpixs[offset].g == centerc && cpixs[offset].b == centerc);
  }

  rgb_image_delete(&base);
  rgb_image_delete(&kernel_test);
  float_image_delete(&kernel);
  
  return TEST_SUCCESS;
}

bool gauss_kernel_test(){

  int width = 15, height = 21;
  rgb_image * base = rgb_image_new(width, height);
  rgb_image * kernel_test = rgb_image_new(base->width / 2, base->height / 2);
  float_image * kernel = float_image_new(7, 7);
  
  *rgb_image_at(base, width / 2, height / 2) = t_rgb_new(100, 100, 100);
  *rgb_image_at(base, width / 2 + 1, height / 2) = t_rgb_new(100, 100, 100);
  *rgb_image_at(base, width / 2 + 3, height / 2 + 1) = t_rgb_new(100, 100, 100);
  
  float_image_gaussian_kernel(kernel, 0.9);
  rgb_image_conv_kernel(base, kernel_test, kernel, true);
  
  rgb_image_delete(&base);
  rgb_image_delete(&kernel_test);
  float_image_delete(&kernel);
  
  return TEST_SUCCESS;
}

bool vr_video_test(){
  TEST(vec_image_average_test);
  TEST(vec_image_median_test);
  TEST(window_function_test);
  TEST(compress_scalespace_test);
  //TEST(distance_field_test_3k);
  //TEST(optical_flow_test);
  //TEST(optical_flow_single_scale_test);
  //TEST(scale_vec_test);
  //TEST(test_visualize_flow);
  //TEST(optical_flow_single_scale_test2);
  
  TEST(image_kernel_test);
  TEST(gauss_kernel_test);
  TEST(optical_flow_single_scale_test3);
  return TEST_SUCCESS;
}
