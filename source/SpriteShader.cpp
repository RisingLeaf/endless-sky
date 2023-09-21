/* SpriteShader.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "SpriteShader.h"

#include "Files.h"
#include "Point.h"
#include "Screen.h"
#include "Shader.h"
#include "Sprite.h"

#include <sstream>
#include <vector>

#ifdef ES_GLES
// ES_GLES always uses the shader, not this, so use a dummy value to compile.
// (the correct value is usually 0x8E46, so don't use that)
#define GL_TEXTURE_SWIZZLE_RGBA 0xBEEF
#endif

using namespace std;

namespace {
	Shader shader;
	int32_t scaleI;
	int32_t texI;
	int32_t swizzleMaskI;
	int32_t useSwizzleMaskI;
	int32_t frameI;
	int32_t frameCountI;
	int32_t positionI;
	int32_t transformI;
	int32_t blurI;
	int32_t clipI;
	int32_t alphaI;
	int32_t swizzlerI;

	uint32_t vao;
	uint32_t vbo;

	const int SWIZZLES = 29;
}

// Initialize the shaders.
void SpriteShader::Init()
{

	static const string vertexCode = Files::Read(Files::Data() + "shaders/Sprite.vert");
	static const string fragmentCode = Files::Read(Files::Data() + "shaders/Sprite.frag");

	shader = Shader(vertexCode.c_str(), fragmentCode.c_str());

	scaleI = shader.Uniform("scale");
	texI = shader.Uniform("tex");
	frameI = shader.Uniform("frame");
	frameCountI = shader.Uniform("frameCount");
	positionI = shader.Uniform("position");
	transformI = shader.Uniform("transform");
	blurI = shader.Uniform("blur");
	clipI = shader.Uniform("clip");
	alphaI = shader.Uniform("alpha");
	swizzlerI = shader.Uniform("swizzler");
	swizzleMaskI = shader.Uniform("swizzleMask");
	useSwizzleMaskI = shader.Uniform("useSwizzleMask");

	// Generate the vertex data for drawing sprites.
	glGenVertexArrays(1, &vao);
	ESG_BindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	GLfloat vertexData[] = {
		-.5f, -.5f,
		-.5f,  .5f,
		 .5f, -.5f,
		 .5f,  .5f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(shader.Attrib("vert"));
	glVertexAttribPointer(shader.Attrib("vert"), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);

	// unbind the VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	ESG_BindVertexArray(0);
}



void SpriteShader::Draw(const Sprite *sprite, const Point &position, float zoom, int swizzle, float frame)
{
	if(!sprite)
		return;

	Bind();
	Add(Prepare(sprite, position, zoom, swizzle, frame));
	Unbind();
}



SpriteShader::Item SpriteShader::Prepare(const Sprite *sprite, const Point &position,
	float zoom, int swizzle, float frame)
{
	if(!sprite)
		return {};

	Item item;
	item.texture = sprite->Texture();
	item.swizzleMask = sprite->SwizzleMask();
	item.frame = frame;
	item.frameCount = sprite->Frames();
	// Position.
	item.position[0] = static_cast<float>(position.X());
	item.position[1] = static_cast<float>(position.Y());
	// Rotation (none) and scale.
	item.transform[0] = sprite->Width() * zoom;
	item.transform[3] = sprite->Height() * zoom;
	// Swizzle.
	item.swizzle = swizzle;

	return item;
}



void SpriteShader::Bind()
{
	ESG_BindShader(shader.Object());
	ESG_BindVertexArray(vao);

	GLfloat scale[2] = {2.f / Screen::Width(), -2.f / Screen::Height()};
	ESG_Uniform2fv(scaleI, scale);
}



void SpriteShader::Add(const Item &item, bool withBlur)
{
	ESG_Uniform1i(texI, 0);
	ESG_BindTexture(GL_TEXTURE_2D_ARRAY, item.texture);

	ESG_Uniform1i(swizzleMaskI, 1);
	// Don't mask full color swizzles that always apply to the whole ship sprite.
	ESG_Uniform1i(useSwizzleMaskI, item.swizzle == 27 || item.swizzleMask == 28 ? 0 : item.swizzleMask);
	glActiveTexture(GL_TEXTURE1);
	ESG_BindTexture(GL_TEXTURE_2D_ARRAY, item.swizzleMask);
	glActiveTexture(GL_TEXTURE0);

	ESG_Uniform1f(frameI, item.frame);
	ESG_Uniform1f(frameCountI, item.frameCount);
	ESG_Uniform2fv(positionI, item.position);
	glUniformMatrix2fv(transformI, 1, false, item.transform);
	// Special case: check if the blur should be applied or not.
	static const float UNBLURRED[2] = {0.f, 0.f};
	ESG_Uniform2fv(blurI, withBlur ? item.blur : UNBLURRED);
	ESG_Uniform1f(clipI, item.clip);
	ESG_Uniform1f(alphaI, item.alpha);

	// Bounds check for the swizzle value:
	int swizzle = (static_cast<size_t>(item.swizzle) >= SWIZZLES ? 0 : item.swizzle);
	// Set the color swizzle.
	ESG_Uniform1i(swizzlerI, swizzle);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}



void SpriteShader::Unbind()
{
	// Reset the swizzle.
	ESG_Uniform1i(swizzlerI, 0);

	ESG_BindVertexArray(0);
	ESG_BindShader(0);
}
