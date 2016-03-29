vec_image * vec_image_scaleup(const vec_image * v, const int new_width, const int new_height);
void average_sample(const vec_image * vec_in, vec_image * vec_out);
void median_sample(const vec_image * vec_in, vec_image * vec_out);
rgb_image * rgb_image_blur_subsample(rgb_image * img, float scale);
