vec_image * vec_image_scaleup(const vec_image * v, const int new_width, const int new_height);
void average_sample(const vec_image * vec_in, vec_image * vec_out, int window_size);
void median_sample(const vec_image * vec_in, vec_image * vec_out, int window_size);
void vec_image_gauss(const vec_image * in, vec_image * out, int window_size, float sigma);
void rgb_image_gauss(const rgb_image * in, rgb_image * out, int window_size, float sigma);

rgb_image * rgb_image_blur_subsample(rgb_image * img, float scale);
void vec_image_apply_kernel(const vec_image * img_in, vec_image * img_out,
			    int kernel_width, int kernel_height,
			    vec2 (* f)(int item_cnt, vec2 * samples));

void vec_image_apply_kernel2(const vec_image * img_in, vec_image * img_out,
			    int kernel_width, int kernel_height,
			     vec2 (* f)(vec2 * samples, int x, int y, int w, int h));

void vec_image_apply2(const vec_image * img1, const vec_image * img2, vec_image * img_out,
		      vec2 (* f)(vec2 v1, vec2 v2));
void rgb_image_conv_kernel(const rgb_image * src, rgb_image * dst, const float_image * kernel, bool normalize);

void float_image_gaussian_kernel(float_image * kernel, float sigma);

