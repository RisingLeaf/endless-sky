/* FrameBuffer.cpp
Copyright (c) 2023 by RisingLeaf

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "FrameBuffer.h"
#include "GameWindow.h"
#include "Screen.h"

#include <map>



namespace {
	std::map<std::string, int> textureStorage;
}



FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &frameBuffer);
	glDeleteTextures(1, &texture);
}



void FrameBuffer::DestroyFrameBuffer()
{
	glDeleteFramebuffers(1, &frameBuffer);
	glDeleteTextures(1, &texture);
}



void FrameBuffer::CreateFrameBuffer()
{
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	const GLenum buffers[] { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffers);
}



void FrameBuffer::CreateTextureAttachment(int _width, int _height)
{
	width = _width;
	height = _height;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, // target, mipmap level, internal format,
		width, height, 1,          // width, height, depth,
		0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);   // border, input format, data type, data.
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, 0);
}



void FrameBuffer::BindFrameBuffer() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0, 0, width, height);
	Screen::ActivateFrameBufferOverwrite(width, height);
}



void FrameBuffer::UnbindCurrentFrameBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GameWindow::AdjustViewport();
	Screen::DeactivateFrameBufferOverwrite();
}



void FrameBuffer::StoreTexture(std::string id, int texture)
{
	textureStorage[id] = texture;
}



int FrameBuffer::GetTexture(std::string id)
{
	return textureStorage[id];
}
