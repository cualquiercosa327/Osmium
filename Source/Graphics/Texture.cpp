#include <Graphics/Texture.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
using namespace Osm;

Texture::Texture(const std::string& filename) : Resource(RESOURCE_TYPE_TEXTURE)
{
	_resourcePath = filename;

	Texture::Reload();
}

Texture::Texture(int width, int height)
	: Resource(RESOURCE_TYPE_TEXTURE)
	, _texture(0)
	, _width(width)
	, _height(height)
{}

Texture::~Texture()
{
	if (_texture)
		glDeleteTextures(1, &_texture);
}

void Texture::CreateGLTextureWithData(GLubyte* data, bool genMipMaps)
{	
	if (_texture)
		glDeleteTextures(1, &_texture);

	glGenTextures(1, &_texture);											// Gen    

	glBindTexture(GL_TEXTURE_2D, _texture);                                 // Bind

	if (genMipMaps)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);    // Minmization
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);                   // Minmization

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);                       // Magnification


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(
		GL_TEXTURE_2D,						// What (target)
		0,									// Mip-map level
		GL_RGBA,		                    // Internal format
		_width,								// Width
		_height,							// Height
		0,									// Border
		GL_RGBA,							// Format (how to use)
		GL_UNSIGNED_BYTE,					// Type   (how to intepret)
		data);								// Data

	if (genMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::Reload()
{
	GLubyte* data = stbi_load(_resourcePath.c_str(), &_width, &_height, &_channels, 4);

	if (data == nullptr)
	{
		LOG("Image %s could not be loaded!", _resourcePath.c_str());
	}
	else
	{
		CreateGLTextureWithData(data, true);
		stbi_image_free(data);
	}
}
