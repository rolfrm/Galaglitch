bool vec_image_median_test();
bool optical_flow_single_scale_test();
bool optical_flow_test();
bool scale_vec_test();
bool test_visualize_flow();
bool test_scalespace_calc();
bool optical_flow_single_scale_test3();
bool optical_flow_single_scale_test2();
void optical_flow_init();
bool compress_scalespace_test();
void construct_scalespace(rgb_image ** ss, rgb_image * img, int count);
vec2 save_pred(const vec_image * pred, const char * path);

void visualize_flow(rgb_image * im1, rgb_image * im2, vec_image * v1_2, vec_image * v2_1);
