#ifndef _VIDEO_SURFACE_TEXTURE_HPP_
#define _VIDEO_SURFACE_TEXTURE_HPP_

#include <chrono>
#include <memory>
#include <string>
#include <vulkan/vulkan.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

class BufferMgr;
class Texture;

class VideoSurfaceTexture {
public:
	VideoSurfaceTexture(const std::string &filename, int depth = 1, int depthColumn = 0, bool loop = true);
	~VideoSurfaceTexture();

	bool isValid() const {
		return valid;
	}

	Texture &getTexture();
	Texture *getBigTexture() {
		return nullptr;
	}

	void getDimensions(int &width, int &height) const;
	void getDimensions(int &width, int &height, int &depth) const;
	void recordUpdate(VkCommandBuffer cmd);

private:
	bool open();
	bool decodeNextFrame();
	bool rewind();
	void close();

	std::string filename;
	bool loop = true;
	bool valid = false;
	bool firstUpload = true;
	bool pendingUpload = false;

	int width = 1;
	int height = 1;
	int atlasWidth = 1;
	int atlasHeight = 1;
	int depth = 1;
	int depthColumn = 1;
	int videoStream = -1;
	double frameRate = 25.0;
	std::chrono::steady_clock::time_point nextFrameTime;
	std::chrono::steady_clock::duration frameDuration = std::chrono::milliseconds(40);

	AVFormatContext *formatContext = nullptr;
	AVCodecContext *codecContext = nullptr;
	const AVCodec *codec = nullptr;
	AVFrame *decodedFrame = nullptr;
	AVFrame *rgbaFrame = nullptr;
	AVPacket *packet = nullptr;
	SwsContext *swsContext = nullptr;

	std::unique_ptr<BufferMgr> stagingBuffer;
	VkDeviceSize stagingOffset = 0;
	std::unique_ptr<Texture> texture;
};

#endif
