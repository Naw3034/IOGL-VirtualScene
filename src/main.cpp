#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Application.h"

int main(int argc, char** argv)
{
	AppOptions options;
	for (int i = 1; i < argc; ++i)
	{
		auto next = [&]() -> const char* { return (i + 1 < argc) ? argv[++i] : ""; };
		if (!std::strcmp(argv[i], "--demo")) options.demo = true;
		else if (!std::strcmp(argv[i], "--record")) options.recordFile = next();
		else if (!std::strcmp(argv[i], "--fps")) options.fps = std::atoi(next());
		else if (!std::strcmp(argv[i], "--start")) options.startTime = (float)std::atof(next());
		else if (!std::strcmp(argv[i], "--end")) options.endTime = (float)std::atof(next());
		else if (!std::strcmp(argv[i], "--fullscreen")) options.fullscreen = true;
		else if (!std::strcmp(argv[i], "--size")) std::sscanf(next(), "%dx%d", &options.width, &options.height);
		else
		{
			std::cout << "Unknown option " << argv[i] << "\n"
				"Usage: VirtualScene [--demo] [--record file.mp4] [--size WxH] [--fps N] [--start s] [--end s] [--fullscreen]"
				<< std::endl;
			return 1;
		}
	}

	Application app;
	if (!app.init(options))
		return -1;
	return app.run();
}
