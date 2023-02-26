//
// Created by Adrien COURNAND on 14/02/2023.
//

#include <cstdint>
#include <cstdlib>

#include "SFML/Graphics.hpp"

#include "glm/vec3.hpp"
#include "glm/geometric.hpp"

#include "common.hpp"

#ifdef WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

using DefaultFpType = float;

using vec3 = glm::vec<3,DefaultFpType,glm::defaultp>;
using point3 = glm::vec<3,DefaultFpType,glm::defaultp>;

using color3 = glm::vec3;

constexpr color3 red {1.0,0,0};
constexpr color3 green {0.0,1.0,0.0};
constexpr color3 blue {0.0,0.0,1.0};
constexpr color3 white = red + green + blue;
constexpr color3 black {0};

struct Ray
{
	Ray(const point3 o = { 0,0,0}, const vec3 d = {0,0,0}): origin(o), direction(d) {}
	point3 constexpr operator[](const DefaultFpType at) { return origin+at*direction; }
	const point3 origin;
	const vec3 direction;
};

constexpr size_t RT_CAMERA_WIDTH = 1920/3;
constexpr size_t RT_CAMERA_HEIGHT = 1080/3;
constexpr size_t RT_CAMERA_PIXEL_COUNT = RT_CAMERA_WIDTH * RT_CAMERA_HEIGHT;

static sf::Texture* texture;
static Pixel* pixels;

static constexpr DefaultFpType CAMERA_LENGTH = 3.0;
static constexpr point3 CAMERA_POSITION { 0,0,-CAMERA_LENGTH };
static constexpr vec3 CAMERA_VIEWPORT_SIZE {1,1,0};
static constexpr point3 CAMERA_PIXEL_SIZE { CAMERA_VIEWPORT_SIZE.x / (DefaultFpType)RT_CAMERA_WIDTH, CAMERA_VIEWPORT_SIZE.y / (DefaultFpType)RT_CAMERA_HEIGHT,0 };
//Center of the upper left pixel
static constexpr vec3 CAMERA_VIEWPORT_UPPER_LEFT = CAMERA_VIEWPORT_SIZE * vec3{-0.5,0.5,0};// + vec3{1.0,-1.0,0} * CAMERA_PIXEL_SIZE * (DefaultFpType)0.5;

static bool hitSphere(const point3 center, const DefaultFpType radius, const Ray& r)
{
	const vec3 oc = r.origin - center;
	auto a = glm::dot(r.direction,r.direction);
	auto b = 2.0*glm::dot(oc,r.direction);
	auto c = glm::dot(oc,oc) - radius*radius;
	auto discriminant = b*b - 4.0*a*c;
	return discriminant > 0;
}

static color3 rayColor(const Ray& r)
{
	return hitSphere({0,0,1},1.0,r) ? red : black;
	//if(hitSphere({0,0,0},1.0,r))
	//{ return red; }
	//const vec3 n = glm::normalize(r.direction);
	//float t = 0.5*(n.y + 1.0);
	//return (1.0f-t)*color3(0.5,0.7,1.0) + t*color3(1.0,1.0,1.0);
}

extern "C" DLL_EXPORT sf::Texture* hotReloadDraw(DrawInfo* info)
{
	info->pixelBufferSize = RT_CAMERA_PIXEL_COUNT*sizeof(pixels[0]);
	info->scaleFactor = 1.5;

	for (size_t y = 0; y < RT_CAMERA_HEIGHT; y += 1)
	{
		for (size_t x = 0; x < RT_CAMERA_WIDTH; x += 1)
		{
			const point3 pixelPos = CAMERA_VIEWPORT_UPPER_LEFT + vec3{ x*CAMERA_PIXEL_SIZE.x,-y*CAMERA_PIXEL_SIZE.y,0 };
			const vec3 rayDirection = pixelPos - CAMERA_POSITION;
			const vec3 rayOrigin = CAMERA_POSITION;
			Ray r (rayOrigin,rayDirection);
			const color3 color = (float)0xFF*rayColor(r);

			const size_t pixelIndex = RT_CAMERA_WIDTH*y+x;
			pixels[pixelIndex].red = color.r;
			pixels[pixelIndex].green = color.g;
			pixels[pixelIndex].blue = color.b;
			pixels[pixelIndex].alpha = 0xFF;
		}
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


