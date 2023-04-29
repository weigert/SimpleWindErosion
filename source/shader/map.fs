#version 430 core

in vec2 ex_Tex;

uniform sampler2D windmap;

out vec4 fragColor;

void main(){
  fragColor = texture(windmap, ex_Tex);
}
