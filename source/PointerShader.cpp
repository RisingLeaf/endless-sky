/* PointerShader.cpp
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

#include "PointerShader.h"

#include "Color.h"
#include "Files.h"
#include "Point.h"
#include "Screen.h"
#include "Shader.h"

#include <stdexcept>

using namespace std;

namespace {
	Shader shader;
	int32_t scaleI;
	int32_t centerI;
	int32_t angleI;
	int32_t sizeI;
	int32_t offsetI;
	int32_t colorI;

	uint32_t vao;
	uint32_t vbo;
}



void PointerShader::Init()
{
	static const string vertexCode = Files::Read(Files::Data() + "shaders/Pointer.vert");
	static const string fragmentCode = Files::Read(Files::Data() + "shaders/Pointer.frag");

	shader = Shader(vertexCode.c_str(), fragmentCode.c_str());

	scaleI = shader.Uniform("scale");
	centerI = shader.Uniform("center");
	angleI = shader.Uniform("angle");
	sizeI = shader.Uniform("size");
	offsetI = shader.Uniform("offset");
	colorI = shader.Uniform("color");

	// Generate the vertex data for drawing sprites.
	glGenVertexArrays(1, &vao);
	ESG_BindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	GLfloat vertexData[] = {
		0.f, 0.f,
		0.f, 1.f,
		1.f, 0.f,
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(shader.Attrib("vert"));
	glVertexAttribPointer(shader.Attrib("vert"), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);

	// unbind the VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	ESG_BindVertexArray(0);
}



void PointerShader::Draw(const Point &center, const Point &angle,
	float width, float height, float offset, const Color &color)
{
	Bind();

	Add(center, angle, width, height, offset, color);

	Unbind();
}



void PointerShader::Bind()
{
	if(!shader.Object())
		throw runtime_error("PointerShader: Bind() called before Init().");

	ESG_BindShader(shader.Object());
	ESG_BindVertexArray(vao);

	GLfloat scale[2] = {2.f / Screen::Width(), -2.f / Screen::Height()};
	ESG_Uniform2fv(scaleI, scale);
}



void PointerShader::Add(const Point &center, const Point &angle,
	float width, float height, float offset, const Color &color)
{
	GLfloat c[2] = {static_cast<float>(center.X()), static_cast<float>(center.Y())};
	ESG_Uniform2fv(centerI, c);

	GLfloat a[2] = {static_cast<float>(angle.X()), static_cast<float>(angle.Y())};
	ESG_Uniform2fv(angleI, a);

	GLfloat size[2] = {width, height};
	ESG_Uniform2fv(sizeI, size);

	ESG_Uniform1f(offsetI, offset);

	glUniform4fv(colorI, 1, color.Get());

	glDrawArrays(GL_TRIANGLES, 0, 3);
}



void PointerShader::Unbind()
{
	ESG_BindVertexArray(0);
	ESG_BindShader(0);
}
