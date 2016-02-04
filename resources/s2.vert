#version 410
// x: angle, y: distance
layout(location = 0) in vec2 vert;

uniform vec2 offset;
uniform vec2 scale;
out float d;
uniform float falloff;
void main(){
  // if falloff is 0.1, the max distance is 10
  //float d2 = min(1.0 / falloff , vert.y);	
  
  vec2 v = vec2(sin(vert.x), cos(vert.x)) * vert.y;
  d= vert.y * falloff;
  gl_Position = vec4((v + offset) * scale,0, 1);
}
