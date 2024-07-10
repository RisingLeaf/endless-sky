/* RingShader.cpp
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

#include "RingShader.h"

#include "Color.h"
#include "Files.h"
#include "pi.h"
#include "Point.h"
#include "Screen.h"
#include "Shader.h"

#include <stdexcept>

using namespace std;

namespace {
	Shader shader;
	int32_t scaleI;
	int32_t positionI;
	int32_t radiusI;
	int32_t widthI;
	int32_t angleI;
	int32_t startAngleI;
	int32_t dashI;
	int32_t colorI;

	uint32_t vao;
	uint32_t vbo;
}



void RingShader::Init()
{
	string vertexCode = Files::Read(Files::Data() + "shaders/Ring.vert");
	string fragmentCode = Files::Read(Files::Data() + "shaders/Ring.frag");
	ESG::ParseShader(vertexCode);
	ESG::ParseShader(fragmentCode);

	shader = Shader(vertexCode.c_str(), fragmentCode.c_str());

	scaleI = shader.Uniform("scale");
	positionI = shader.Uniform("position");
	radiusI = shader.Uniform("radius");
	widthI = shader.Uniform("width");
	angleI = shader.Uniform("angle");
	startAngleI = shader.Uniform("startAngle");
	dashI = shader.Uniform("dash");
	colorI = shader.Uniform("color");

	// Generate the vertex data for drawing sprites.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	float vertexData[] = {
		-1.f, -1.f,
		-1.f,  1.f,
		 1.f, -1.f,
		 1.f,  1.f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(shader.Attrib("vert"));
	glVertexAttribPointer(shader.Attrib("vert"), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

	// unbind the VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



void RingShader::Draw(const Point &pos, float out, float in, const Color &color)
{
	float width = .5f * (1.f + out - in) ;
	Draw(pos, out - width, width, 1.f, color);
}



void RingShader::Draw(const Point &pos, float radius, float width, float fraction,
	const Color &color, float dash, float startAngle)
{
	Bind();

	Add(pos, radius, width, fraction, color, dash, startAngle);

	Unbind();
}



void RingShader::Bind()
{
	if(!shader.Object())
		throw runtime_error("RingShader: Bind() called before Init().");

	glUseProgram(shader.Object());
	glBindVertexArray(vao);

	float scale[2] = {2.f / Screen::Width(), -2.f / Screen::Height()};
	glUniform2fv(scaleI, 1, scale);
}



void RingShader::Add(const Point &pos, float out, float in, const Color &color)
{
	float width = .5f * (1.f + out - in) ;
	Add(pos, out - width, width, 1.f, color);
}



void RingShader::Add(const Point &pos, float radius, float width, float fraction,
	const Color &color, float dash, float startAngle)
{
	float position[2] = {static_cast<float>(pos.X()), static_cast<float>(pos.Y())};
	glUniform2fv(positionI, 1, position);

	glUniform1f(radiusI, radius);
	glUniform1f(widthI, width);
	glUniform1f(angleI, fraction * 2. * PI);
	glUniform1f(startAngleI, startAngle * TO_RAD);
	glUniform1f(dashI, dash ? 2. * PI / dash : 0.);

	glUniform4fv(colorI, 1, color.Get());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}



void RingShader::Unbind()
{
	glBindVertexArray(0);
	glUseProgram(0);
}
