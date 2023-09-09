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

#include "Screen.h"
#include "Shader.h"

#include <stdexcept>

namespace {
	Shader shader;
	GLint scaleI;
	GLint centerI;
	GLint sizeI;
	GLint colorI;

	GLint useGradientI;
	GLint colorBI;
	GLint gradientStartI;
	GLint gradientEndI;


	GLuint vao;
	GLuint vbo;
}



void FillShader::Init()
{
	static const char *vertexCode =
		"// vertex fill shader\n"
		"uniform vec2 scale;\n"
		"uniform vec2 center;\n"
		"uniform vec2 size;\n"

		"in vec2 vert;\n"

		"out vec2 fragCenter;\n"

		"void main() {\n"
		"  fragCenter = center;"
		"  gl_Position = vec4((center + vert * size) * scale, 0, 1);\n"
		"}\n";

	static const char *fragmentCode =
		"// fragment fill shader\n"
		"precision mediump float;\n"
		"uniform vec4 color;\n"

		"uniform int useGradient;\n"
		"uniform vec4 colorB;\n"
		"uniform vec2 gradientStart;\n"
		"uniform vec2 gradientEnd;\n"

		"in vec2 fragCenter;\n"

		"out vec4 finalColor;\n"

		"void main() {\n"
		"  if(useGradient > 0)\n"
		"  {\n"
		"    float gradientStartDistance = length(gradientStart - gl_FragCoord.xy);\n"
		"    float gradientEndDistance = length(gradientEnd - gl_FragCoord.xy);\n"
		"    float mixFactor = gradientStartDistance / (gradientStartDistance + gradientEndDistance);\n"
		"    float mixFactorB = gradientEndDistance / (gradientStartDistance + gradientEndDistance);\n"
		"    vec4 interpolatedColor = color * mixFactor + colorB *(1. - mixFactor);\n"
		"    finalColor = vec4(mixFactor, mixFactorB, 0, 10. * interpolatedColor.a);\n"
		"  }\n"
		"  else\n"
		"    finalColor = color;\n"
		"}\n";

	shader = Shader(vertexCode, fragmentCode);
	scaleI = shader.Uniform("scale");
	centerI = shader.Uniform("center");
	sizeI = shader.Uniform("size");
	colorI = shader.Uniform("color");

	useGradientI = shader.Uniform("useGradient");
	colorBI = shader.Uniform("colorB");
	gradientStartI = shader.Uniform("gradientStart");
	gradientEndI = shader.Uniform("gradientEnd");

	// Generate the vertex data for drawing sprites.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	GLfloat vertexData[] = {
		-.5f, -.5f,
		 .5f, -.5f,
		-.5f,  .5f,
		 .5f,  .5f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(shader.Attrib("vert"));
	glVertexAttribPointer(shader.Attrib("vert"), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);

	// unbind the VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



void FillShader::Fill(const Point &center, const Point &size, const Color &color, Point gradientStart, Point gradientEnd, const Color &colorB)
{
	if(!shader.Object())
		throw std::runtime_error("FillShader: Draw() called before Init().");

	glUseProgram(shader.Object());
	glBindVertexArray(vao);

	GLfloat scale[2] = {2.f / Screen::Width(), -2.f / Screen::Height()};
	glUniform2fv(scaleI, 1, scale);

	GLfloat centerV[2] = {static_cast<float>(center.X()), static_cast<float>(center.Y())};
	glUniform2fv(centerI, 1, centerV);

	GLfloat sizeV[2] = {static_cast<float>(size.X()), static_cast<float>(size.Y())};
	glUniform2fv(sizeI, 1, sizeV);

	glUniform4fv(colorI, 1, color.Get());

	if(gradientStart.X() > 0. || true)
	{
		glUniform1i(useGradientI, 42);
		glUniform4fv(colorBI, 1, colorB.Get());

		GLfloat gradientStartV[2] = {static_cast<float>(gradientStart.X()), static_cast<float>(gradientStart.Y())};
		glUniform2fv(gradientStartI, 1, gradientStartV);

		GLfloat gradientEndV[2] = {static_cast<float>(gradientEnd.X()), static_cast<float>(gradientEnd.Y())};
		glUniform2fv(gradientEndI, 1, gradientEndV);
	}
	else
		glUniform1i(useGradientI, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
	glUseProgram(0);
}
