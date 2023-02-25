//
// Created by Adrien COURNAND on 14/02/2023.
//

#include <cstdint>
#include <cstdlib>

#include "SFML/Graphics.hpp"

#include "common/log.hpp"
#include "common.hpp"

#ifdef WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

constexpr size_t RT_CAMERA_WIDTH = 100;
constexpr size_t RT_CAMERA_HEIGHT = 200;
constexpr size_t RT_CAMERA_PIXEL_COUNT = RT_CAMERA_WIDTH * RT_CAMERA_HEIGHT;

static sf::Texture* texture;
static Pixel* pixels;

extern "C" DLL_EXPORT sf::Texture* hotReloadDraw(DrawInfo* info)
{
	info->pixelBufferSize = RT_CAMERA_PIXEL_COUNT*sizeof(pixels[0]);
	for (size_t i = 0; i < RT_CAMERA_PIXEL_COUNT; i += 1)
	{
		pixels[i].red = 0;
		pixels[i].green = 0xFF;
		pixels[i].blue = 0;
		pixels[i].alpha = 0xFF;
	}

	texture->update((const uint8_t*)pixels);
	return texture;
}

__attribute__((constructor)) void init(void)
{
	texture = new sf::Texture();
	pixels = new Pixel [RT_CAMERA_PIXEL_COUNT];
	texture->create(RT_CAMERA_WIDTH,RT_CAMERA_HEIGHT);
}

__attribute__((destructor)) void clean(void)
{
	delete[] pixels;
	delete texture;
}


