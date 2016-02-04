#version 410
out vec4 color;
in float d;
void main(){
  float d2 = d / 200.0;
  color = vec4(1.0 - d2);
}