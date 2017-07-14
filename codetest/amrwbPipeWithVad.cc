#include "wavwriter.h"
#include "fvadfunc.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <dec_if.h>

const int sizes[] = {17, 23, 32, 36, 40, 46, 50, 58, 60, 5, -1, -1, -1, -1, -1, 0};

int main(int argc, char* argv[])
{
	FILE* in;
	char header[9];
	int n;
	if (argc < 3)
	{
		fprintf(stderr, "%s in.amr [out.wav|-(for pipe)]\n", argv[0]);
		return 1;
	}
	in = fopen(argv[1], "rb");
	if (!in)
	{
		perror(argv[1]);
		return 1;
	}
	n = fread(header, 1, 9, in);
	if (n != 9 || memcmp(header, "#!AMR-WB\n", 9))
	{
		fprintf(stderr, "Bad header\n");
		return 1;
	}

	void* amr = D_IF_init();
	std::vector<uint8_t> data;
	int circletimes = 0;
	while (1)
	{
		uint8_t buffer[500], littleendian[640], *ptr;
		int size, i;
		int16_t outbuffer[320];
		n = fread(buffer, 1, 1, in);
		if (n <= 0)
			break;
		size = sizes[(buffer[0] >> 3) & 0x0f];
		if (size < 0)
			break;
		n = fread(buffer + 1, 1, size, in);
		if (n != size)
			break;
		D_IF_decode(amr, buffer, outbuffer, 0);
		ptr = littleendian;
		for (i = 0; i < 320; ++i)
		{
			*ptr++ = (outbuffer[i] >> 0) & 0xff;
			*ptr++ = (outbuffer[i] >> 8) & 0xff;
		}
		for (i = 0; i < 640; ++i)
			data.push_back(littleendian[i]);
		++circletimes;
	}
	fprintf(stderr, "data length: %ld\t circle times: %d\n", data.size(), circletimes);
	int data_length = data.size();
	uint8_t *in_data = &data[0];
	std::vector<uint8_t> out_data;
	if (!process_vad(in_data, 16000, 3, 30, 1, 16, data_length, out_data))
	{
		fprintf(stderr, "processing vad failed");
		return 1;
	}
	int out_data_length = out_data.size();
	void *output = wav_write_open(argv[2], 16000, 16, 1, out_data_length);
	if (!output)
	{
		fprintf(stderr, "Unable to open %s\n", argv[2]);
		return 1;
	}
	const unsigned char* out_ptr = reinterpret_cast<const unsigned char*>(&out_data[0]);
	wav_write_data(output, out_ptr, out_data_length);
	fprintf(stderr, "Finish write data into %s\n", argv[2]);
	fclose(in);
	D_IF_exit(amr);
	data.clear();
	return 0;
}
