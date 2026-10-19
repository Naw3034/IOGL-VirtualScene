//-----------------------------------------------------------------------------
// Resolves the path of a resource (shader, model, texture).
// The program can be launched from the project folder, from the build folder
// or from an IDE: we try the working directory first, then the source folder.
//-----------------------------------------------------------------------------
#ifndef PATHS_H
#define PATHS_H

#include <fstream>
#include <string>

#ifndef IOGL_SOURCE_DIR
#define IOGL_SOURCE_DIR ""
#endif

inline std::string resourcePath(const std::string& relative)
{
	const std::string candidates[] = { relative, "../" + relative, std::string(IOGL_SOURCE_DIR) + relative };
	for (const std::string& c : candidates)
	{
		std::ifstream f(c);
		if (f.good())
			return c;
	}
	return relative;
}

#endif // PATHS_H
