#include "fvadfunc.h"
#include <iostream>


bool process_vad(uint8_t* indata, int sample_rate, int vad_mode, int frame_ms, int channels, int bits_per_sample, int data_length, std::vector<uint8_t> &out_data)
{
	int16_t* buf1 = NULL;
	std::vector<int16_t*> out_buf;
	uint8_t* data = indata;
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
	int index = 1;
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

bool process_vad(double* indata, int sample_rate, int vad_mode, int frame_ms, int channels, int bits_per_sample, int data_length, std::vector<double> &out_data)
{
	fprintf(stderr, "It's in process_vad double function.\n");
	int16_t* buf1 = NULL;
	std::vector<double*> out_buf;
	double *data = indata;
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
	int index = 1;
	while (static_cast<int>(index * frame_len) < data_length)
	{
		for (size_t i = 0; i < frame_len; ++i)
			buf1[i] = static_cast<int16_t>(data[i]);
		int vadres = fvad_process(vad, buf1, frame_len);
		if (vadres < 0)
		{
			fprintf(stderr, "Vad processing failed\n");
			return false;
		}
		if (vadres)
		{
			out_buf.emplace_back();
			out_buf.back() = (double*)malloc(frame_len * sizeof(double));
			memcpy(out_buf.back(), data, frame_len * sizeof(double));
		}
		memset(buf1, 0, frame_len);
		data += frame_len;
		++index;
	}
	fprintf(stderr, "complete vad filter, malloc memory size: %ld\n", out_buf.size() * frame_len * sizeof(double));
	for (size_t i = 0; i < out_buf.size(); ++i)
	{
		double* temp_buf_ptr = reinterpret_cast<double*>(out_buf[i]);
		for (size_t j = 0; j < frame_len; ++j)
			out_data.push_back(temp_buf_ptr[j]);
	}
	fprintf(stderr, "out buf size:%ld, frame_len:%ld, out data length:%ld\n", out_buf.size(), frame_len, out_data.size());
	fprintf(stderr, "finish vad algorithm\n");
	for (size_t i = 0; i < out_buf.size(); ++i)
		free(out_buf[i]);
	out_buf.clear();
	return true;
}

bool process_vad(float* indata, int sample_rate, int vad_mode, int frame_ms, int channels, int bits_per_sample, int data_length, std::vector<float> &out_data)
{
	fprintf(stderr, "It's in process_vad float function.\n");
	int16_t* buf1 = NULL;
	std::vector<float*> out_buf;
	float *data = indata;
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
	int index = 1;
	while (static_cast<int>(index * frame_len < data_length))
	{
		for (size_t i = 0; i < frame_len; ++i)
			buf1[i] = static_cast<int16_t>(data[i]);
		int vadres = fvad_process(vad, buf1, frame_len);
		if (vadres < 0)
		{
			fprintf(stderr, "Vad processing failed\n");
			return false;
		}
		if (vadres)
		{
			out_buf.emplace_back();
			out_buf.back() = (float*)malloc(frame_len * sizeof(float));
			memcpy(out_buf.back(), data, frame_len * sizeof(float));
		}
		memset(buf1, 0, frame_len);
		data += frame_len;
		++index;
	}
	fprintf(stderr, "complete vad filter, malloc memory size: %ld\n", out_buf.size() * frame_len * sizeof(float));
	for (size_t i = 0; i < out_buf.size(); ++i)
	{
		float* temp_buf_ptr = out_buf[i];
		for (size_t j = 0; j < frame_len; ++j)
			out_data.push_back(temp_buf_ptr[j]);
	}
	fprintf(stderr, "out buf size:%ld, frame_len:%ld, out data length:%ld\n", out_buf.size(), frame_len, out_data.size());
	fprintf(stderr, "finish vad algorithm\n");
	for (size_t i = 0; i < out_buf.size(); ++i)
		free(out_buf[i]);
	out_buf.clear();
	return true;
}
