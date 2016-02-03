#version 410
// x: angle, y: distance
layout(location = 0) in vec2 vert;

uniform vec2 offset;
uniform vec2 scale;
void main(){
  vec2 v = vec2(sin(vert.x), cos(vert.x)) * vert.y;
  gl_Position = vec4((v + offset) * scale,0, 1);
}
