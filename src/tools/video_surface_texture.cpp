#include "tools/video_surface_texture.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

extern "C" {
#include <libavutil/imgutils.h>
}

#include "EntityCore/Core/BufferMgr.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "EntityCore/Resource/Texture.hpp"
#include "tools/log.hpp"

namespace {
bool metadataHasAlpha(const AVDictionary *metadata)
{
	if (!metadata)
		return false;
	const AVDictionaryEntry *entry = av_dict_get(metadata, "alpha_mode", nullptr, 0);
	if (!entry)
		entry = av_dict_get(metadata, "alpha", nullptr, 0);
	return entry && std::string(entry->value) == "1";
}
}

VideoSurfaceTexture::VideoSurfaceTexture(const std::string &_filename, int _depth, int _depthColumn, bool _loop)
	: filename(_filename), loop(_loop), depth(std::max(_depth, 1))
{
	if (_depthColumn > 0) {
		depthColumn = _depthColumn;
	} else {
		depthColumn = 1 << (static_cast<int>(std::log2(depth)) / 2);
	}
	depthColumn = std::max(depthColumn, 1);

	valid = open();
	if (!valid) {
		close();
		return;
	}

	if (!decodeNextFrame()) {
		cLog::get()->write("VideoSurfaceTexture: no decodable frame in " + filename, LOG_TYPE::L_ERROR);
		close();
		valid = false;
		return;
	}

	nextFrameTime = std::chrono::steady_clock::now() + frameDuration;
}

VideoSurfaceTexture::~VideoSurfaceTexture()
{
	close();
}

bool VideoSurfaceTexture::open()
{
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
	av_register_all();
#endif
	avformat_network_init();

	if (avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr) != 0) {
		cLog::get()->write("VideoSurfaceTexture: can't open " + filename, LOG_TYPE::L_ERROR);
		return false;
	}
	if (avformat_find_stream_info(formatContext, nullptr) < 0) {
		cLog::get()->write("VideoSurfaceTexture: can't read stream info from " + filename, LOG_TYPE::L_ERROR);
		return false;
	}

	for (unsigned int i = 0; i < formatContext->nb_streams; ++i) {
		if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = static_cast<int>(i);
			break;
		}
	}
	if (videoStream < 0) {
		cLog::get()->write("VideoSurfaceTexture: no video stream in " + filename, LOG_TYPE::L_ERROR);
		return false;
	}

	AVStream *stream = formatContext->streams[videoStream];
	const bool metadataAlpha = metadataHasAlpha(stream->metadata) || metadataHasAlpha(formatContext->metadata);
	if (stream->codecpar->codec_id == AV_CODEC_ID_VP9) {
		codec = avcodec_find_decoder_by_name("libvpx-vp9");
		if (codec) {
			cLog::get()->write("VideoSurfaceTexture: using libvpx-vp9 decoder for " + filename, LOG_TYPE::L_INFO);
		} else {
			cLog::get()->write("VideoSurfaceTexture: libvpx-vp9 decoder not found, using default VP9 decoder for " + filename, LOG_TYPE::L_WARNING);
		}
	}
	if (!codec)
		codec = avcodec_find_decoder(stream->codecpar->codec_id);
	if (!codec) {
		cLog::get()->write("VideoSurfaceTexture: unsupported codec for " + filename, LOG_TYPE::L_ERROR);
		return false;
	}

	codecContext = avcodec_alloc_context3(codec);
	if (!codecContext || avcodec_parameters_to_context(codecContext, stream->codecpar) < 0) {
		cLog::get()->write("VideoSurfaceTexture: can't create codec context for " + filename, LOG_TYPE::L_ERROR);
		return false;
	}
	if (metadataAlpha && stream->codecpar->codec_id == AV_CODEC_ID_VP9)
		codecContext->pix_fmt = AV_PIX_FMT_YUVA420P;
	if (avcodec_open2(codecContext, codec, nullptr) < 0) {
		cLog::get()->write("VideoSurfaceTexture: can't open codec for " + filename, LOG_TYPE::L_ERROR);
		return false;
	}
	const bool hasAlpha = metadataAlpha || codecContext->pix_fmt == AV_PIX_FMT_YUVA420P ||
	                      codecContext->pix_fmt == AV_PIX_FMT_RGBA ||
	                      codecContext->pix_fmt == AV_PIX_FMT_BGRA ||
	                      codecContext->pix_fmt == AV_PIX_FMT_ARGB ||
	                      codecContext->pix_fmt == AV_PIX_FMT_ABGR;
	if (hasAlpha)
		cLog::get()->write("VideoSurfaceTexture: video alpha channel detected for " + filename, LOG_TYPE::L_INFO);

	atlasWidth = std::max(codecContext->width, 1);
	atlasHeight = std::max(codecContext->height, 1);
	if (depth % depthColumn != 0) {
		cLog::get()->write("VideoSurfaceTexture: depth is not divisible by depthColumn for " + filename, LOG_TYPE::L_ERROR);
		return false;
	}
	const int depthRows = depth / depthColumn;
	if (atlasWidth % depthColumn != 0 || atlasHeight % depthRows != 0) {
		cLog::get()->write("VideoSurfaceTexture: video atlas size is incompatible with depth layout for " + filename, LOG_TYPE::L_ERROR);
		return false;
	}
	width = atlasWidth / depthColumn;
	height = atlasHeight / depthRows;

	AVRational guessedRate = av_guess_frame_rate(formatContext, stream, nullptr);
	if (guessedRate.num > 0 && guessedRate.den > 0)
		frameRate = guessedRate.num / static_cast<double>(guessedRate.den);
	if (frameRate <= 0.0)
		frameRate = 25.0;
	frameDuration = std::chrono::steady_clock::duration(static_cast<int64_t>(
		std::chrono::steady_clock::period::den / (std::chrono::steady_clock::period::num * frameRate)));

	decodedFrame = av_frame_alloc();
	rgbaFrame = av_frame_alloc();
	packet = av_packet_alloc();
	if (!decodedFrame || !rgbaFrame || !packet)
		return false;

	swsContext = sws_getContext(atlasWidth, atlasHeight, codecContext->pix_fmt,
	                            atlasWidth, atlasHeight, AV_PIX_FMT_RGBA,
	                            SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!swsContext) {
		cLog::get()->write("VideoSurfaceTexture: can't create RGBA converter for " + filename, LOG_TYPE::L_ERROR);
		return false;
	}

	const uint32_t frameSize = atlasWidth * atlasHeight * 4;
	stagingBuffer = std::make_unique<BufferMgr>(*VulkanMgr::instance,
	                                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                                            0, frameSize, "Planet video texture staging");
	auto staging = stagingBuffer->acquireBuffer(frameSize);
	stagingOffset = staging.offset;
	av_image_fill_arrays(rgbaFrame->data, rgbaFrame->linesize,
	                     static_cast<uint8_t *>(stagingBuffer->getPtr(staging)),
	                     AV_PIX_FMT_RGBA, atlasWidth, atlasHeight, 1);
	if (depth == 1) {
		rgbaFrame->data[0] += rgbaFrame->linesize[0] * (atlasHeight - 1);
		rgbaFrame->linesize[0] = -rgbaFrame->linesize[0];
	}

	texture = std::make_unique<Texture>(*VulkanMgr::instance, TextureInfo{
		.width = width,
		.height = height,
		.depth = depth,
		.depthColumn = depthColumn,
		.content = nullptr,
		.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.type = (depth > 1) ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D,
		.name = filename,
	});
	texture->use();

	return true;
}

void VideoSurfaceTexture::close()
{
	texture.reset();
	stagingBuffer.reset();
	if (swsContext) {
		sws_freeContext(swsContext);
		swsContext = nullptr;
	}
	if (packet) {
		av_packet_free(&packet);
		packet = nullptr;
	}
	if (rgbaFrame) {
		av_frame_free(&rgbaFrame);
		rgbaFrame = nullptr;
	}
	if (decodedFrame) {
		av_frame_free(&decodedFrame);
		decodedFrame = nullptr;
	}
	if (codecContext) {
		avcodec_free_context(&codecContext);
		codecContext = nullptr;
	}
	if (formatContext) {
		avformat_close_input(&formatContext);
		formatContext = nullptr;
	}
}

bool VideoSurfaceTexture::rewind()
{
	if (av_seek_frame(formatContext, videoStream, 0, AVSEEK_FLAG_BACKWARD) < 0)
		return false;
	avcodec_flush_buffers(codecContext);
	return true;
}

bool VideoSurfaceTexture::decodeNextFrame()
{
	while (av_read_frame(formatContext, packet) >= 0) {
		if (packet->stream_index != videoStream) {
			av_packet_unref(packet);
			continue;
		}

		const int sendResult = avcodec_send_packet(codecContext, packet);
		av_packet_unref(packet);
		if (sendResult < 0)
			continue;

		while (avcodec_receive_frame(codecContext, decodedFrame) == 0) {
			sws_scale(swsContext, decodedFrame->data, decodedFrame->linesize,
			          0, atlasHeight, rgbaFrame->data, rgbaFrame->linesize);
			pendingUpload = true;
			return true;
		}
	}

	if (!loop || !rewind())
		return false;
	return decodeNextFrame();
}

Texture &VideoSurfaceTexture::getTexture()
{
	return *texture;
}

void VideoSurfaceTexture::getDimensions(int &_width, int &_height) const
{
	_width = width;
	_height = height;
}

void VideoSurfaceTexture::getDimensions(int &_width, int &_height, int &_depth) const
{
	_width = width;
	_height = height;
	_depth = depth;
}

void VideoSurfaceTexture::recordUpdate(VkCommandBuffer cmd)
{
	if (!valid || !texture)
		return;

	const auto now = std::chrono::steady_clock::now();
	if (!firstUpload && now >= nextFrameTime) {
		if (decodeNextFrame()) {
			do {
				nextFrameTime += frameDuration;
			} while (nextFrameTime < now);
		}
	}

	if (!pendingUpload)
		return;

	VkImageMemoryBarrier before {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
		firstUpload ? 0U : VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		firstUpload ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
		texture->getImage(),
		{texture->getAspect(), 0, 1, 0, 1}};
	vkCmdPipelineBarrier(cmd,
		firstUpload ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, nullptr, 0, nullptr, 1, &before);

	VkBufferImageCopy region {};
	region.bufferOffset = stagingOffset;
	region.bufferRowLength = (depth == 1) ? 0 : static_cast<uint32_t>(atlasWidth);
	region.bufferImageHeight = 0;
	region.imageSubresource = {texture->getAspect(), 0, 0, 1};
	region.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(depth)};
	if (depth == 1) {
		vkCmdCopyBufferToImage(cmd, stagingBuffer->getBuffer(), texture->getImage(),
		                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	} else {
		std::vector<VkBufferImageCopy> regions;
		regions.reserve(depthColumn);
		region.imageExtent.depth /= depthColumn;
		for (int i = 0; i < depthColumn; ++i) {
			regions.push_back(region);
			region.bufferOffset += width * 4;
			region.imageOffset.z += region.imageExtent.depth;
		}
		vkCmdCopyBufferToImage(cmd, stagingBuffer->getBuffer(), texture->getImage(),
		                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		                       static_cast<uint32_t>(regions.size()), regions.data());
	}

	VkImageMemoryBarrier after {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
		texture->getImage(),
		{texture->getAspect(), 0, 1, 0, 1}};
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
	                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	                     0, 0, nullptr, 0, nullptr, 1, &after);

	firstUpload = false;
	pendingUpload = false;
}
