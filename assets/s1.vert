#version 410
layout(location = 0) in vec2 vert;
out vec2 uv;
uniform vec2 offset;
void main(){
  uv = vert * 0.5 + 0.5;
  gl_Position = vec4((vert + offset) * 0.1,0, 1);
}