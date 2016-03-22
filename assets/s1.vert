#version 410
layout(location = 0) in vec2 vert;
uniform vec2 offset;
void main(){
  
  gl_Position = vec4((vert + offset) * 0.1,0, 1);
}