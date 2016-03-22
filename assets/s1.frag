#version 410
out vec4 color;
in vec2 uv;
in float d;
uniform sampler2D tex;
void main(){
  color = texture(tex, uv);
} 
