#version 410
layout(location = 0) in vec3 vert;
void main(){
  gl_Position = vec4(vert.x * 0.1,vert.y * 0.1,0, 1);
}
