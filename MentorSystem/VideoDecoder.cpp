#include "VideoDecoder.h"

#include <iostream>

VideoDecoder::VideoDecoder(void)
{
	std::cout << "TODO: initialize VideoDecoder" << std::endl;
}

bool VideoDecoder::decode(char* in_buffer, int in_buffer_size, cv::Mat* out_mat) {
	bool returnValue = false;

	char* bytesToDecode = in_buffer;
	int lengthOfArray = in_buffer_size;

	if (lengthOfArray > 0) {

		this->_decoder_pkt.size = lengthOfArray;
		this->_decoder_pkt.data = (uint8_t*) bytesToDecode;

		int len;
		int got_frame;

		len = avcodec_decode_video2(this->_decoder_c, this->_decoder_frame, &got_frame, &this->_decoder_pkt);

		if (len < 0) {
			std::cout << "error while decoding frame" << std::endl;
			returnValue = false;
		}

		if (got_frame) {

			cv::Mat* decodedMat = out_mat;

			uint8_t* rgb24Data = decodedMat->data;
			uint8_t * outData[1] = { rgb24Data };	// rgb24 has one plane
			int outLinesize[1] = { 3*this->_decoderWidthPixels };	// rgb stride

			std::cout << "decodedMatColsnRows: " << decodedMat->cols << "," << decodedMat->rows << std::endl;
			std::cout << "outLinesize[0]: " << outLinesize[0] << std::endl;
			std::cout << "_decoder_frame->height: " << this->_decoder_frame->height << std::endl;
			std::cout << "_decoder_frame->linesize: " << this->_decoder_frame->linesize << std::endl;
			std::cout << "_decoderHeightPixels: " << this->_decoderHeightPixels << std::endl;
				//this->_decoder_frame->linesize
				//this->_decoderHeightPixels

			sws_scale(this->_decoder_sws, this->_decoder_frame->data, this->_decoder_frame->linesize, 0, this->_decoderHeightPixels, outData, outLinesize);

			returnValue = true;

		}

	} else {
		std::cout << "bytes to decode were empty, doing nothing" << std::endl;
	}

	return returnValue;
}

void VideoDecoder::destroyDecoder() {
	std::cout << "destroyDecoder" << std::endl;

	avcodec_close(this->_decoder_c);
	av_free(this->_decoder_c);
	av_frame_free(&this->_decoder_frame);
	printf("\n");
}

void VideoDecoder::initDecoder(int width, int height) {
	std::cout << "in initDecoder" << std::endl;

	this->_decoder_c = NULL;

	this->_decoderWidthPixels = width;
	this->_decoderHeightPixels = height;

	/* register all the codecs */
	avcodec_register_all();

	//enum AVCodecID codec_id = AV_CODEC_ID_MPEG4;
	enum AVCodecID codec_id = AV_CODEC_ID_MJPEG;

	AVCodec *decoder_codec;

	uint8_t inbuf[INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];

	av_init_packet(&_decoder_pkt);

	/* set end of buffer to 0 (this ensures that no overreading happens for damaged mpeg streams) */
	memset(inbuf + INBUF_SIZE, 0, FF_INPUT_BUFFER_PADDING_SIZE);

	/* find the mpeg1 video decoder */
	decoder_codec = avcodec_find_decoder(codec_id);
	if (!decoder_codec) {
		std::cout << "Codec not found" << std::endl;
		exit(1);
	}

	_decoder_c = avcodec_alloc_context3(decoder_codec);
	if (!_decoder_c) {
		std::cout << "Could not allocate video codec context" << std::endl;
		exit(1);
	}

	if(decoder_codec->capabilities&CODEC_CAP_TRUNCATED) {
		_decoder_c->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */
	}

	/* For some codecs, such as msmpeg4 and mpeg4, width and height
		   MUST be initialized there because this information is not
		   available in the bitstream. */

	/* open it */
	if (avcodec_open2(_decoder_c, decoder_codec, NULL) < 0) {
		std::cout << "Could not open codec" << std::endl;
		exit(1);
	}

	// here is where we would open the file from a filename, but we aren't reading from a file

	_decoder_frame = av_frame_alloc();
	if (!_decoder_frame) {
		std::cout << "Could not allocate video frame" << std::endl;
		exit(1);
	}

	_decoder_sws = sws_getContext(this->_decoderWidthPixels, this->_decoderHeightPixels, AV_PIX_FMT_YUVJ420P, this->_decoderWidthPixels, this->_decoderHeightPixels, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, 0, 0, 0);

	// here is where we would decode each frame

	std::cout << "done initing decoder" << std::endl;
}

VideoDecoder::~VideoDecoder(void)
{
}
