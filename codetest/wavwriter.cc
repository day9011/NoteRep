#include "wavwriter.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


struct wav_writer
{
	FILE* wav_output;
	int data_length;
	int sample_rate;
	int bits_per_sample;
	int channels;
};

static void write_syn(struct wav_writer* ww, const char* syn)
{
	fputc(syn[0], ww->wav_output);
	fputc(syn[1], ww->wav_output);
	fputc(syn[2], ww->wav_output);
	fputc(syn[3], ww->wav_output);
}

static void write_int32(struct wav_writer* ww, int value)
{
	fputc((value >> 0) & 0xff, ww->wav_output);
	fputc((value >> 8) & 0xff, ww->wav_output);
	fputc((value >> 16) & 0xff, ww->wav_output);
	fputc((value >> 24) & 0xff, ww->wav_output);
}

static void write_int16(struct wav_writer* ww, int value)
{
	fputc((value >> 0) & 0xff, ww->wav_output);
	fputc((value >> 8) & 0xff, ww->wav_output);
}

static void write_header(struct wav_writer* ww, int data_length)
{
	int bytes_per_frame, bytes_per_sec;
	write_syn(ww, "RIFF");
	write_int32(ww, 4 + 8 + 16 + 8 + data_length);
	write_syn(ww, "WAVE");
	write_syn(ww, "fmt ");
	write_int32(ww, 16);
	bytes_per_frame = ww->bits_per_sample / 8 * (ww->channels);
	bytes_per_sec = bytes_per_frame * (ww->sample_rate);
	write_int16(ww, 1);
	write_int16(ww, ww->channels);
	write_int32(ww, ww->sample_rate);
	write_int32(ww, bytes_per_sec);
	write_int16(ww, bytes_per_frame);
	write_int16(ww, ww->bits_per_sample);
	write_syn(ww, "data");
	write_int32(ww, data_length);
}

void* wav_write_open(const char* filename, int sample_rate, int bits_per_sample, int channels, int data_length)
{
	struct wav_writer* ww= (struct wav_writer*)malloc(sizeof(*ww));
	memset(ww, 0, sizeof(*ww));
	if (filename[0] == '-')
		ww->wav_output = stdout;
	else
		ww->wav_output = fopen(filename, "wb");
	if (ww->wav_output == NULL)
	{
		free(ww);
		return NULL;
	}
	ww->data_length = data_length;
	ww->sample_rate = sample_rate;
	ww->bits_per_sample = bits_per_sample;
	ww->channels = channels;
	write_header(ww, ww->data_length);
	return ww;
}

void wav_write_data(void *obj, const unsigned char* data, int length)
{
	struct wav_writer* ww = (struct wav_writer*)obj;
	if (ww->wav_output == NULL)
		return;
	fwrite(data, length, 1, ww->wav_output);
}


