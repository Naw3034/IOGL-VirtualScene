//-----------------------------------------------------------------------------
// Video capture: reads back every rendered frame (glReadPixels) and pipes it
// into FFmpeg, which encodes an H.264 MP4. FFmpeg must be in the PATH.
// Frames are produced with a fixed time step, so the video is perfectly
// smooth whatever the speed of the computer.
//-----------------------------------------------------------------------------
#ifndef VIDEO_RECORDER_H
#define VIDEO_RECORDER_H

#include <cstdio>
#include <string>
#include <vector>

class VideoRecorder
{
public:
	~VideoRecorder();

	bool open(const std::string& file, int width, int height, int fps);
	void captureFrame();	// reads the default framebuffer
	void close();
	bool isOpen() const { return mPipe != nullptr; }

private:
	FILE* mPipe = nullptr;
	int mWidth = 0, mHeight = 0;
	std::vector<unsigned char> mPixels;
};

#endif // VIDEO_RECORDER_H
