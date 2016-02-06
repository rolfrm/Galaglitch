#version 410
precision highp float;
out vec4 color;
uniform vec3 in_color;
uniform float falloff;
in highp float d;
const float pattern[64] = float[64](
    0, 32,  8, 40,  2, 34, 10, 42,   
    48, 16, 56, 24, 50, 18, 58, 26,  
    12, 44,  4, 36, 14, 46,  6, 38,  
    60, 28, 52, 20, 62, 30, 54, 22,  
    3, 35, 11, 43,  1, 33,  9, 41,   
    51, 19, 59, 27, 49, 17, 57, 25,
    15, 47,  7, 39, 13, 45,  5, 37,
    63, 31, 55, 23, 61, 29, 53, 21 );

void main(){
   float offset = mod(floor(gl_FragCoord.x),8) + mod(floor(gl_FragCoord.y), 8) * 8;
   float alpha = 1000 / (1000 + 20 * d + 0.2 * pow(d, 2));
   
   float dither = (pattern[int(offset)] / 64 - 0.5) / 64.0 / length(in_color);
   //dither = 0;
   color = vec4(in_color * vec3(1 + dither * 3), alpha);
}