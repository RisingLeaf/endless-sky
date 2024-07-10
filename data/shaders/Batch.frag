// fragment batch shader
precision mediump float;

precision mediump sampler2DArray;
//?opengl layout(binding = 1) uniform sampler2DArray tex;
//?vulkan layout(set = 1, binding = 1) uniform sampler2DArray tex;
uniform float frameCount;

layout(location = 0) in vec3 fragTexCoord;
layout(location = 1) in float fragAlpha;
layout(location = 0) out vec4 finalColor;

void main() {

  float first = floor(fragTexCoord.z);
  float second = mod(ceil(fragTexCoord.z), frameCount);
  float fade = fragTexCoord.z - first;
  finalColor = mix(
    texture(tex, vec3(fragTexCoord.xy, first)),
    texture(tex, vec3(fragTexCoord.xy, second)), fade);
  finalColor *= vec4(fragAlpha);

}
