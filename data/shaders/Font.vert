// vertex font shader

uniform vec2 scale;
uniform vec2 position;
uniform int glyph;
uniform float aspect;

in vec2 vert;
in vec2 corner;

out vec2 texCoord;

void main() {
  texCoord = vec2((float(glyph) + corner.x) / 98.f, corner.y);
  gl_Position = vec4((aspect * vert.x + position.x) * scale.x, (vert.y + position.y) * scale.y, 0.f, 1.f);
}