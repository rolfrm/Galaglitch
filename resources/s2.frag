#version 410
out vec4 color;
uniform vec3 in_color;
uniform float falloff;
in float d;

void main(){
   color = vec4(in_color,clamp(1.0 - d, 0, 1));
}