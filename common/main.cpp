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
static sf::Texture* (*hotReloadDraw)(DrawInfo*) = nullptr;

static DrawInfo drawInfo;

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

static constexpr size_t TEXT_STAT_BUFFER_SIZE = 1024;
char statTextBuffer[TEXT_STAT_BUFFER_SIZE];

int main(void)
{
	sf::RenderWindow w (sf::VideoMode(1080,720),"Raytracing", sf::Style::Close | sf::Style::Titlebar);
	w.setVerticalSyncEnabled(true);

	sf::Texture* texture = nullptr;
	sf::Font font;
	font.loadFromFile(ASSETS_PATH "/font.ttf");

	sf::Clock rtClock;
	sf::Clock c;

	float elapsedSinceLastSecond = 0;
	unsigned int frameCounter = 0;
	unsigned int fpsValue = 0;
	int printBufferUsage = 1;
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

		rtClock.restart();
		if (hotReloadDraw)
		{ texture = hotReloadDraw(&drawInfo); }
		const float RTMsTime = rtClock.restart().asMicroseconds() / 1000.0;

		w.clear();
		sf::Sprite sprite (*texture);
		sprite.scale(drawInfo.scaleFactor,drawInfo.scaleFactor);
		w.draw(sprite);

		const float frameMsTime = c.restart().asMicroseconds() / 1000.0;
		const float rtTimeRatio = RTMsTime / frameMsTime * 100.0;

		frameCounter += 1;
		elapsedSinceLastSecond += frameMsTime;

		if (elapsedSinceLastSecond >= 1000.0)
		{
			fpsValue = frameCounter;
			frameCounter = 0;
			elapsedSinceLastSecond -= 1000.0;
		}

		printBufferUsage = snprintf(statTextBuffer,TEXT_STAT_BUFFER_SIZE,
				 "FPS: %d - print buffer : %d / %d\n"
				 "%.2fms/%.2fms (%3.2f%%)\n"
				 "pixel buffer: %.3fko\n"
				 "scale: %.2f",
				 fpsValue,printBufferUsage,TEXT_STAT_BUFFER_SIZE,RTMsTime,frameMsTime,rtTimeRatio,drawInfo.pixelBufferSize / 1024.f,drawInfo.scaleFactor);
		w.draw(sf::Text(statTextBuffer,font));

		w.display();
	}

	dylib_Unload(hotReloadModule);
	return EXIT_SUCCESS;
}
