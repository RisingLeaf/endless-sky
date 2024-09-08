// fragment sprite shader
precision mediump float;

uniform sampler2DArray tex;
uniform sampler2DArray swizzleMask;
uniform int useSwizzleMask;
uniform float frame;
uniform float frameCount;
uniform vec2 blur;
uniform int swizzler;
uniform float alpha;

const int range = 5;

in vec2 fragTexCoord;

out vec4 finalColor;

void main()
{
  float first = floor(frame);
  float second = mod(ceil(frame), frameCount);
  float fade = frame - first;
  vec4 color;
  if(blur.x == 0.f && blur.y == 0.f)
  {
    if(fade != 0.f)
      color = mix(
        texture(tex, vec3(fragTexCoord, first)),
        texture(tex, vec3(fragTexCoord, second)), fade);
    else
      color = texture(tex, vec3(fragTexCoord, first));
  }
  else
  {
    color = vec4(0., 0., 0., 0.);
    const float divisor = float(range * (range + 2) + 1);
    for(int i = -range; i <= range; ++i)
    {
      float scale = float(range + 1 - abs(i)) / divisor;
      vec2 coord = fragTexCoord + (blur * float(i)) / float(range);
      if(fade != 0.f)
        color += scale * mix(
          texture(tex, vec3(coord, first)),
          texture(tex, vec3(coord, second)), fade);
      else
        color += scale * texture(tex, vec3(coord, first));
    }
  }
  vec4 swizzleColor;
  switch (swizzler) {
    case 0:
      swizzleColor = color.rgba;
      break;
    case 1:
      swizzleColor = color.rbga;
      break;
    case 2:
      swizzleColor = color.grba;
      break;
    case 3:
      swizzleColor = color.brga;
      break;
    case 4:
      swizzleColor = color.gbra;
      break;
    case 5:
      swizzleColor = color.bgra;
      break;
    case 6:
      swizzleColor = color.gbba;
      break;
    case 7:
      swizzleColor = color.rbba;
      break;
    case 8:
      swizzleColor = color.rgga;
      break;
    case 9:
      swizzleColor = color.bbba;
      break;
    case 10:
      swizzleColor = color.ggga;
      break;
    case 11:
      swizzleColor = color.rrra;
      break;
    case 12:
      swizzleColor = color.bbga;
      break;
    case 13:
      swizzleColor = color.bbra;
      break;
    case 14:
      swizzleColor = color.ggra;
      break;
    case 15:
      swizzleColor = color.bgga;
      break;
    case 16:
      swizzleColor = color.brra;
      break;
    case 17:
      swizzleColor = color.grra;
      break;
    case 18:
      swizzleColor = color.bgba;
      break;
    case 19:
      swizzleColor = color.brba;
      break;
    case 20:
      swizzleColor = color.grga;
      break;
    case 21:
      swizzleColor = color.ggba;
      break;
    case 22:
      swizzleColor = color.rrba;
      break;
    case 23:
      swizzleColor = color.rrga;
      break;
    case 24:
      swizzleColor = color.gbga;
      break;
    case 25:
      swizzleColor = color.rbra;
      break;
    case 26:
      swizzleColor = color.rgra;
      break;
    case 27:
      swizzleColor = vec4(color.b, 0.f, 0.f, color.a);
      break;
    case 28:
      swizzleColor = vec4(0.f, 0.f, 0.f, color.a);
      break;
  }
  if(useSwizzleMask > 0)
  {
    float factor = texture(swizzleMask, vec3(fragTexCoord, first)).r;
    color = color * factor + swizzleColor * (1.0 - factor);
  }
  else
    color = swizzleColor;
  finalColor = color * alpha;
}
