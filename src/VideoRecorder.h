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
	void captureFrame();
	void close();
	bool isOpen() const { return mPipe != nullptr; }

private:
	FILE* mPipe = nullptr;
	int mWidth = 0, mHeight = 0;
	std::vector<unsigned char> mPixels;
};

#endif
