#include "fvadfunc.h"
#include <iostream>


bool process_vad(uint8_t* indata, int sample_rate, int vad_mode, int frame_ms, int channels, int bits_per_sample, int data_length, std::vector<uint8_t> &out_data)
{
	int16_t* buf1 = NULL;
	std::vector<int16_t*> out_buf;
	uint8_t* data = indata;
	// std::cout << "indata:" << std::endl;
	// for (int i = 0; i < data_length; ++i)
	//     fprintf(stdout, " %x", indata[i]);
	// std::cout << std::endl;
	Fvad* vad = NULL;
	size_t frame_len =  (size_t)sample_rate / 1000 * frame_ms;
	vad = fvad_new();
	if (!vad)
	{
		fprintf(stderr, "out of memory\n");
		return false;
	}
	if (fvad_set_mode(vad, vad_mode) < 0)
	{
	    fprintf(stderr, "invalid vad mode.\n");
		return false;
	}
	if (fvad_set_sample_rate(vad, sample_rate) < 0)
	{
	    fprintf(stderr, "invalid sample rate: %d Hz\n", sample_rate);
		return false;
	}

	fprintf(stderr, "config fvad successfully!\n");

	if (!(buf1 = (int16_t*)malloc(frame_len * sizeof(int16_t))))
	{
		fprintf(stderr, "failed to allocate buffers\n");
		return false;
	}
	fprintf(stderr, "malloc memory successfully\n");
	int index = 0;
	while (static_cast<int>(index * frame_len * sizeof(int16_t)) < data_length)
	{
		memcpy(buf1, data, frame_len * sizeof(int16_t));
		int vadres = fvad_process(vad, buf1, frame_len);
		if (vadres < 0)
		{
			fprintf(stderr, "Vad processing failed\n");
			return false;
		}
		if (vadres)
		{
			out_buf.emplace_back();
			out_buf.back() = (int16_t*)malloc(frame_len * sizeof(int16_t));
			memcpy(out_buf.back(), buf1, frame_len * sizeof(int16_t));
		}
		memset(buf1, 0, frame_len);
		data += frame_len * sizeof(int16_t);
		++index;
	}
	fprintf(stderr, "complete vad filter, malloc memory size: %ld\n", out_buf.size() * frame_len * sizeof(int16_t));
	for (size_t i = 0; i < out_buf.size(); ++i)
	{
		uint8_t* temp_buf_ptr = reinterpret_cast<uint8_t*>(out_buf[i]);
		for (size_t j = 0; j < (frame_len * sizeof(int16_t)); ++j)
			out_data.push_back(temp_buf_ptr[j]);
	}
	fprintf(stderr, "out buf size:%ld, frame_len:%ld, out data length:%ld\n", out_buf.size(), frame_len, out_data.size());
	fprintf(stderr, "finish vad algorithm\n");
	for (size_t i = 0; i < out_buf.size(); ++i)
		free(out_buf[i]);
	out_buf.clear();
	return true;
}
