// fragment font shader
precision mediump float;

uniform sampler2D tex;
uniform vec4 color;

in vec2 texCoord;

out vec4 finalColor;

void main() {
  finalColor = texture(tex, texCoord).a * color;
}