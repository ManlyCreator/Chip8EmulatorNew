#version 330 core

uniform sampler2D texSample;

in vec2 texCoord;

out vec4 fragCol;

void main() {
  float value = texture(texSample, texCoord).r * 255;
  fragCol = vec4(value, value, value, 1.0f);
}
