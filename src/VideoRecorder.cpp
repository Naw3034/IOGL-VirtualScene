//-----------------------------------------------------------------------------
// Video capture through an FFmpeg pipe
//-----------------------------------------------------------------------------
#include "VideoRecorder.h"

#include <iostream>
#include "GL/glew.h"

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

VideoRecorder::~VideoRecorder()
{
	close();
}

bool VideoRecorder::open(const std::string& file, int width, int height, int fps)
{
	mWidth = width;
	mHeight = height;
	mPixels.resize((size_t)width * height * 3);

	// OpenGL images start at the bottom row -> vflip
	std::string cmd = "ffmpeg -y -loglevel error -f rawvideo -pix_fmt rgb24 -s " + std::to_string(width) + "x"
		+ std::to_string(height) + " -r " + std::to_string(fps) + " -i - -vf vflip -c:v libx264 -preset medium"
		+ " -crf 17 -pix_fmt yuv420p -movflags +faststart \"" + file + "\"";
#ifdef _WIN32
	mPipe = popen(cmd.c_str(), "wb");
#else
	mPipe = popen(cmd.c_str(), "w");
#endif
	if (!mPipe)
	{
		std::cerr << "Could not start ffmpeg (is it installed and in the PATH?)" << std::endl;
		return false;
	}
	std::cout << "Recording " << width << "x" << height << " @" << fps << " fps to " << file << std::endl;
	return true;
}

void VideoRecorder::captureFrame()
{
	if (!mPipe) return;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, mWidth, mHeight, GL_RGB, GL_UNSIGNED_BYTE, mPixels.data());
	fwrite(mPixels.data(), 1, mPixels.size(), mPipe);
}

void VideoRecorder::close()
{
	if (mPipe)
	{
		pclose(mPipe);
		mPipe = nullptr;
		std::cout << "Video saved." << std::endl;
	}
}
