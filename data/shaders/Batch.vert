uniform vec2 scale;

layout(location = 0) in vec2 vert;
layout(location = 1) in vec3 texCoord;
layout(location = 2) in float alpha;

layout(location = 0) out vec3 fragTexCoord;
layout(location = 1) out float fragAlpha;

void main() {
  gl_Position = vec4(vert * scale, 0, 1);
  fragTexCoord = texCoord;
  fragAlpha = alpha;
}
