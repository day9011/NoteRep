#ifndef FVADFUNC_H
#define FVADFUNC_H

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sndfile.h>
#include <vector>
extern "C"
{
#include <fvad.h>
}

//Just apply to handle 16 bits_per_sample
bool process_vad(uint8_t* data, int sample_rate, int vad_mode, int frame_ms, int channels, int bits_per_sample, int data_length, std::vector<uint8_t> &out_data);


#endif
