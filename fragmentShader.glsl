#version 330 core

uniform sampler2D texSample;

in vec2 texCoord;

out vec4 fragCol;

void main() {
  fragCol = texture(texSample, texCoord);
}
