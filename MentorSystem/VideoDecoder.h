#pragma once

// added to fix compilation error about UINT64_C not being defined (solution from https://github.com/AutonomyLab/ardrone_autonomy/issues/1 )
#ifndef UINT64_C
#define UINT64_C(c) (c ## ULL)
#endif

#ifdef __cplusplus
extern "C" {
#endif
// ffmpeg libraries are in C not C++, so this may help with proper linking
// https://stackoverflow.com/questions/15625468/libav-linking-error-undefined-references
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixfmt.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#ifdef __cplusplus
}
#endif

#include <opencv2/opencv.hpp>
#define INBUF_SIZE 4096

class VideoDecoder
{
public:
	VideoDecoder(void);
	~VideoDecoder(void);

	void initDecoder(int frameWidth, int frameHeight);

	bool decode(char* in_buffer, int in_buffer_size, cv::Mat* out_mat);

	void destroyDecoder();

private:

	int _decoderWidthPixels;
	int _decoderHeightPixels;

	AVCodecContext *_decoder_c;
	AVFrame *_decoder_frame;
	AVPacket _decoder_pkt;
	SwsContext* _decoder_sws;



	
};

