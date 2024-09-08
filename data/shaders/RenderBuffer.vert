// vertex blit shader
precision mediump float;

uniform vec2 size;
uniform vec2 position;
uniform vec2 scale;

uniform vec2 srcposition;
uniform vec2 srcscale;

in vec2 vert;
out vec2 tpos;
out vec2 vpos;

void main() 
{
  gl_Position = vec4((position + vert * size) * scale, 0, 1);
  vpos = vert + vec2(.5, .5);         // Convert from vertex to texture coordinates.
  vec2 tsize = size * srcscale;       // Convert from screen to texture coordinates.
  vec2 tsrc = srcposition * srcscale; // Convert from screen to texture coordinates.
  tpos = vpos * tsize + tsrc;
  tpos.y = 1.0 - tpos.y;              // Negative is up.
}
