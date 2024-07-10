/* FillShader.cpp
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

#include "FillShader.h"

#include "Color.h"
#include "Files.h"
#include "Point.h"
#include "Screen.h"
#include "Shader.h"

#include <stdexcept>

namespace {
	Shader shader;
	int32_t scaleI;
	int32_t centerI;
	int32_t sizeI;
	int32_t colorI;

	uint32_t vao;
	uint32_t vbo;
}

using namespace std;



void FillShader::Init()
{
	string vertexCode = Files::Read(Files::Data() + "shaders/Fill.vert");
	string fragmentCode = Files::Read(Files::Data() + "shaders/Fill.frag");
	ESG::ParseShader(vertexCode);
	ESG::ParseShader(fragmentCode);

	shader = Shader(vertexCode.c_str(), fragmentCode.c_str());
	scaleI = shader.Uniform("scale");
	centerI = shader.Uniform("center");
	sizeI = shader.Uniform("size");
	colorI = shader.Uniform("color");

	// Generate the vertex data for drawing sprites.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	float vertexData[] = {
		-.5f, -.5f,
		 .5f, -.5f,
		-.5f,  .5f,
		 .5f,  .5f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(shader.Attrib("vert"));
	glVertexAttribPointer(shader.Attrib("vert"), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

	// unbind the VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



void FillShader::Fill(const Point &center, const Point &size, const Color &color)
{
	if(!shader.Object())
		throw std::runtime_error("FillShader: Draw() called before Init().");

	glUseProgram(shader.Object());
	glBindVertexArray(vao);

	float scale[2] = {2.f / Screen::Width(), -2.f / Screen::Height()};
	glUniform2fv(scaleI, 1, scale);

	float centerV[2] = {static_cast<float>(center.X()), static_cast<float>(center.Y())};
	glUniform2fv(centerI, 1, centerV);

	float sizeV[2] = {static_cast<float>(size.X()), static_cast<float>(size.Y())};
	glUniform2fv(sizeI, 1, sizeV);

	glUniform4fv(colorI, 1, color.Get());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
	glUseProgram(0);
}
