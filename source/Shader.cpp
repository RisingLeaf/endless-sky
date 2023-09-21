/* Shader.cpp
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

#include "Shader.h"

#include "Logger.h"

#include <cctype>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

using namespace std;



Shader::Shader(const char *vertex, const char *fragment)
{
	uint32_t vertexShader = Compile(vertex, GL_VERTEX_SHADER);
	uint32_t fragmentShader = Compile(fragment, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	if(!program)
		throw runtime_error("Creating OpenGL shader program failed.");

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);

	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);

	int32_t status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if(status == GL_FALSE)
	{
		int32_t maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
		string error(infoLog.data());
		Logger::LogError(error);

		throw runtime_error("Linking OpenGL shader program failed.");
	}
}



uint32_t Shader::Object() const noexcept
{
	return program;
}



int32_t Shader::Attrib(const char *name) const
{
	int32_t attrib = glGetAttribLocation(program, name);
	if(attrib == -1)
		throw runtime_error("Attribute \"" + string(name) + "\" not found.");

	return attrib;
}



int32_t Shader::Uniform(const char *name) const
{
	int32_t uniform = glGetUniformLocation(program, name);
	if(uniform == -1)
		throw runtime_error("Uniform \"" + string(name) + "\" not found.");

	return uniform;
}



uint32_t Shader::Compile(const char *str, GLenum type)
{
	uint32_t object = glCreateShader(type);
	if(!object)
		throw runtime_error("Shader creation failed.");

	static string version;
	if(version.empty())
	{
		version = "#version ";
		string glsl = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
		bool found = false;
		for(char c : glsl)
		{
			if(!found && !isdigit(c))
			{
				continue;
			}
			if(isspace(c))
				break;
			if(isdigit(c))
			{
				found = true;
				version += c;
			}
		}
		if(glsl.find("GLSL ES") != std::string::npos)
		{
			version += " es";
		}
		version += '\n';
	}
	size_t length = strlen(str);
	vector<GLchar> text(version.length() + length + 1);
	memcpy(&text.front(), version.data(), version.length());
	memcpy(&text.front() + version.length(), str, length);
	text[version.length() + length] = '\0';

	const GLchar *cText = &text.front();
	glShaderSource(object, 1, &cText, nullptr);
	glCompileShader(object);

	int32_t status;
	glGetShaderiv(object, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE)
	{
		string error = version;
		error += string(str, length);

		static const int SIZE = 4096;
		GLchar message[SIZE];
		GLsizei length;

		glGetShaderInfoLog(object, SIZE, &length, message);
		error += string(message, length);
		Logger::LogError(error);
		throw runtime_error("Shader compilation failed.");
	}

	return object;
}
