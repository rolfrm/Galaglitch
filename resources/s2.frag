#version 410
out vec4 color;
uniform vec3 in_color;
uniform float falloff;
in float d;
const float pattern[64] = float[64](
    0, 32,  8, 40,  2, 34, 10, 42,   /* 8x8 Bayer ordered dithering  */
    48, 16, 56, 24, 50, 18, 58, 26,  /* pattern.  Each input pixel   */
    12, 44,  4, 36, 14, 46,  6, 38,  /* is scaled to the 0..63 range */
    60, 28, 52, 20, 62, 30, 54, 22,  /* before looking in this table */
    3, 35, 11, 43,  1, 33,  9, 41,   /* to determine the action.     */
    51, 19, 59, 27, 49, 17, 57, 25,
    15, 47,  7, 39, 13, 45,  5, 37,
    63, 31, 55, 23, 61, 29, 53, 21 );
precision highp float;
void main(){
   float offset = mod(floor(gl_FragCoord.x),8) + mod(floor(gl_FragCoord.y), 8) * 8;
   float dither = pattern[int(offset)] / 256.0 / 64.0 - (1.0 / 128.0);
   //dither = 0;
   color = vec4(in_color + in_color * vec3(dither * 20),clamp(1.0 - d, 0, 1));
}