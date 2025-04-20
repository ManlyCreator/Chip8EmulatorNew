#version 330 core

uniform sampler2D texSample;

in vec2 texCoord;

out vec4 fragCol;

void main() {
  float value = texture(texSample, texCoord).r;
  fragCol = vec4(vec3(value), 1.0f);
}
