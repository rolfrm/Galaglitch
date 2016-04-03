i16 * simple_optical_flow(rgb_image * img1, rgb_image * img2, int window_size);
void optical_flow_2(const rgb_image * img1, const rgb_image * img2,
		    vec_image * pred, const int window_size);
void compress_scalespace(vec_image ** scalespace, int max_scale);
vec2 calc_scalespace_vector(vec_image ** const scalespace, int x, int y, int scale, int * last_significiant_scale);
void optical_flow_3(const rgb_image * img1, const rgb_image * img2,
		    vec_image ** pred_scalespace, float_image * error_img,
		    const int scale,
		    const int window_size);
