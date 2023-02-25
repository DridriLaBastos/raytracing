//
// Created by Adrien COURNAND on 25/02/2023.
//

#include <filesystem>

#include "log.hpp"
#include "common.hpp"

#include "SFML/Window/Event.hpp"

#include "SFML/Graphics.hpp"

static std::filesystem::file_time_type lastReloadTime;
static std::error_code errorCode;
static module_t hotReloadModule = INVALID_MODULE_ID;
static sf::Texture* (*hotReloadDraw)(void) = nullptr;

static void checkAndReloadDrawFunction(void)
{
	const auto lastWritetime = std::filesystem::last_write_time(HOT_RELOAD_LIB_FULL_PATH,errorCode);
	if (errorCode)
	{
		ERROR("Can't get draw library last access time : '%s' (error code %d)", errorCode.message().c_str(),errorCode.value());
		INFO("Last loaded code will be used");
		return;
	}

	if (lastWritetime == lastReloadTime)
	{ return; }

	INFO("Trying reload");
	lastReloadTime = lastWritetime;

	std::filesystem::copy_file(HOT_RELOAD_LIB_FULL_PATH,HOT_RELOAD_LIB_COPY_PATH,std::filesystem::copy_options::overwrite_existing,errorCode);

	if (errorCode)
	{
		ERROR("Cannot copy '%s' to '%s' : '%s' (error code %d)",HOT_RELOAD_LIB_FULL_PATH,HOT_RELOAD_LIB_COPY_PATH,errorCode.message().c_str(),errorCode.value());
		INFO("Last loaded code will be used");
		return;
	}

	const module_t module = dylib_Reload(HOT_RELOAD_LIB_COPY_PATH,hotReloadModule);

	if (module == INVALID_MODULE_ID)
	{
		ERROR("Unable to load module '%s' : '%s'",HOT_RELOAD_LIB_COPY_PATH, dylib_GetErrorStr());
		INFO("Last loaded code will be used");
		return;
	}

	hotReloadModule = module;

	auto* newHotReloadDraw = hotReloadDraw;

	dylib_LoadFunctionWithName("hotReloadDraw",hotReloadModule,(void**)&newHotReloadDraw);

	if (newHotReloadDraw == nullptr)
	{
		ERROR("Unable to reload draw code : '%s'",dylib_GetErrorStr());
		INFO("Last loaded code will be used");
		return;
	}

	hotReloadDraw = newHotReloadDraw;
	INFO("Draw code successfully reloaded");
}

#if 1
int main(void)
{
	sf::RenderWindow w (sf::VideoMode(480,360),"Raytracing",sf::Style::Close | sf::Style::Titlebar);

	sf::Texture* texture = nullptr;

	sf::Clock c;
	while(w.isOpen()) {
		sf::Event event;

		while (w.pollEvent(event)) {
			switch (event.type) {
				case sf::Event::Closed:
					w.close();
					break;

				default:
					break;
			}
		}
		checkAndReloadDrawFunction();

		if (hotReloadDraw)
		{ texture = hotReloadDraw(); }

		w.clear();
		w.draw(sf::Sprite(*texture));

		sf::Time elapsed = c.restart();



		w.display();
	}

	dylib_Unload(hotReloadModule);
	return EXIT_SUCCESS;
}
#else
#include <SFML/Graphics.hpp>

int main()
{
	// Create a window to display the pixel buffer
	sf::RenderWindow window(sf::VideoMode(800, 600), "Pixel Buffer Example");

	// Create a texture to hold the pixel buffer data
	sf::Texture texture;
	texture.create(800, 600);

	// Create a C++ vector to hold the pixel buffer data
	std::vector<sf::Uint32> pixels(800 * 600); // 4 bytes per pixel (RGBA)

	// Fill the pixel buffer with red pixels
	for (int i = 0; i < pixels.size(); i += 1) {
		pixels[i] = 0xFFFFFF00;
	}

	// Update the texture with the pixel data
	texture.update((uint8_t*)pixels.data());

	// Create a sprite and set its texture
	sf::Sprite sprite(texture);

	// Draw the sprite to the window
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
				case sf::Event::Closed:
					window.close();
					break;

				default:
					break;
			}
		}
		window.clear();
		window.draw(sprite);
		window.display();
	}

	return 0;
}
#endif
