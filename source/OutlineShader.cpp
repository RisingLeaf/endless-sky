/* OutlineShader.cpp
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

#include "OutlineShader.h"

#include "Color.h"
#include "Files.h"
#include "Point.h"
#include "Screen.h"
#include "Shader.h"
#include "Sprite.h"

using namespace std;

namespace {
	Shader shader;
	int32_t scaleI;
	int32_t offI;
	int32_t transformI;
	int32_t positionI;
	int32_t frameI;
	int32_t frameCountI;
	int32_t colorI;

	uint32_t vao;
	uint32_t vbo;
}



void OutlineShader::Init()
{
	string vertexCode = Files::Read(Files::Data() + "shaders/Outline.vert");
	string fragmentCode = Files::Read(Files::Data() + "shaders/Outline.frag");
	ESG::ParseShader(vertexCode);
	ESG::ParseShader(fragmentCode);

	shader = Shader(vertexCode.c_str(), fragmentCode.c_str());

	scaleI = shader.Uniform("scale");
	offI = shader.Uniform("off");
	transformI = shader.Uniform("transform");
	positionI = shader.Uniform("position");
	frameI = shader.Uniform("frame");
	frameCountI = shader.Uniform("frameCount");
	colorI = shader.Uniform("color");

	glUseProgram(shader.Object());
	glUniform1i(shader.Uniform("tex"), 0);
	glUseProgram(0);

	// Generate the vertex data for drawing sprites.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	float vertexData[] = {
		-.5f, -.5f, 0.f, 0.f,
		 .5f, -.5f, 1.f, 0.f,
		-.5f,  .5f, 0.f, 1.f,
		 .5f,  .5f, 1.f, 1.f
	};
	constexpr auto stride = 4 * sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(shader.Attrib("vert"));
	glVertexAttribPointer(shader.Attrib("vert"), 2, GL_FLOAT, GL_FALSE, stride, nullptr);

	glEnableVertexAttribArray(shader.Attrib("vertTexCoord"));
	glVertexAttribPointer(shader.Attrib("vertTexCoord"), 2, GL_FLOAT, GL_TRUE,
		stride, reinterpret_cast<const void*>(2 * sizeof(float)));

	// unbind the VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



void OutlineShader::Draw(const Sprite *sprite, const Point &pos, const Point &size,
	const Color &color, const Point &unit, float frame)
{
	glUseProgram(shader.Object());
	glBindVertexArray(vao);

	float scale[2] = {2.f / Screen::Width(), -2.f / Screen::Height()};
	glUniform2fv(scaleI, 1, scale);

	float off[2] = {
		static_cast<float>(.5 / size.X()),
		static_cast<float>(.5 / size.Y())};
	glUniform2fv(offI, 1, off);

	glUniform1f(frameI, frame);
	glUniform1f(frameCountI, sprite->Frames());

	Point uw = unit * size.X();
	Point uh = unit * size.Y();
	float transform[4] = {
		static_cast<float>(-uw.Y()),
		static_cast<float>(uw.X()),
		static_cast<float>(-uh.X()),
		static_cast<float>(-uh.Y())
	};
	glUniformMatrix2fv(transformI, 1, false, transform);

	float position[2] = {
		static_cast<float>(pos.X()), static_cast<float>(pos.Y())};
	glUniform2fv(positionI, 1, position);

	glUniform4fv(colorI, 1, color.Get());

	glBindTexture(GL_TEXTURE_2D_ARRAY, sprite->Texture(unit.Length() * Screen::Zoom() > 50.));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
	glUseProgram(0);
}
