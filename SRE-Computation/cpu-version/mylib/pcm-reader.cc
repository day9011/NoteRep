// feat/wave-reader.cc

// Copyright 2009-2011  Karel Vesely;  Petr Motlicek
//                2013  Florent Masson
//                2013  Johns Hopkins University (author: Daniel Povey)

// See ../../COPYING for clarification regarding multiple authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.

#include <cstdio>
#include <sstream>
#include <vector>

#include "mylib/pcm-reader.h"
#include "base/kaldi-error.h"
#include "base/kaldi-utils.h"

namespace kaldi {

// static
void PCMData::Expect4ByteTag(std::istream &is, const char *expected) {
  char tmp[5];
  tmp[4] = '\0';
  is.read(tmp, 4);
  if (is.fail())
    KALDI_ERR << "PCMData: expected " << expected << ", failed to read anything";
  if (strcmp(tmp, expected))
    KALDI_ERR << "PCMData: expected " << expected << ", got " << tmp;
}

uint32 PCMData::ReadUint32(std::istream &is, bool swap) {
  union {
    char result[4];
    uint32 ans;
  } u;
  is.read(u.result, 4);
  if (swap)
    KALDI_SWAP4(u.result);
  if (is.fail())
    KALDI_ERR << "PCMData: unexpected end of file.";
  return u.ans;
}


uint16 PCMData::ReadUint16(std::istream &is, bool swap) {
  union {
    char result[2];
    int16 ans;
  } u;
  is.read(u.result, 2);
  if (swap)
    KALDI_SWAP2(u.result);
  if (is.fail())
    KALDI_ERR << "PCMData: unexpected end of file.";
  return u.ans;
}

// static
void PCMData::Read4ByteTag(std::istream &is, char *dest) {
  is.read(dest, 4);
  if (is.fail())
    KALDI_ERR << "PCMData: expected 4-byte chunk-name, got read errror";
}

// static
void PCMData::WriteUint32(std::ostream &os, int32 i) {
  union {
    char buf[4];
    int i;
  } u;
  u.i = i;
#ifdef __BIG_ENDIAN__
  KALDI_SWAP4(u.buf);
#endif
  os.write(u.buf, 4);
  if (os.fail())
    KALDI_ERR << "PCMData: error writing to stream.";
}

void PCMData::WriteUint16(std::ostream &os, int16 i) {
  union {
    char buf[2];
    int16 i;
  } u;
  u.i = i;
#ifdef __BIG_ENDIAN__
  KALDI_SWAP2(u.buf);
#endif
  os.write(u.buf, 2);
  if (os.fail())
    KALDI_ERR << "PCMData: error writing to stream.";
}



void PCMData::Read(std::istream &is, long fileLen) {
  data_.Resize(0, 0);  // clear the data

  std::vector<char*> data_pointer_vec;
  std::vector<int> data_size_vec;
  uint32 num_bytes_read = 0;
  uint32 block_align = 2;
  uint32 num_channels = 1;
  uint32 bits_per_sample = 16;
  uint32 swap = 1;
  samp_freq_ = sampFreq;
  uint32 data_chunk_size = fileLen;
  KALDI_LOG << "data size:" << data_chunk_size << " byte";
  for (int32 remain_chunk_size = data_chunk_size; remain_chunk_size > 0;
       remain_chunk_size -= kBlockSize) {
    int32 this_block_size = remain_chunk_size;
    if (kBlockSize < remain_chunk_size)
      this_block_size = kBlockSize;
    char *block_data_vec = new char[this_block_size];
    is.read(block_data_vec, this_block_size);
    num_bytes_read += is.gcount();
    data_size_vec.push_back(is.gcount());
    data_pointer_vec.push_back(block_data_vec);
    if (num_bytes_read < this_block_size)
      break;
  }

  std::vector<char> chunk_data_vec(num_bytes_read);
  uint32 data_address = 0;
  for (int i = 0; i < data_pointer_vec.size(); i++) {
    memcpy(&(chunk_data_vec[data_address]), data_pointer_vec[i],
           data_size_vec[i]);
    delete[] data_pointer_vec[i];
    data_address += data_size_vec[i];
  }

  char *data_ptr = &(chunk_data_vec[0]);
  if (num_bytes_read == 0 && num_bytes_read != data_chunk_size) {
    KALDI_ERR << "PCMData: failed to read data chunk (read no bytes)";
  } else if (num_bytes_read != data_chunk_size) {
    KALDI_ASSERT(num_bytes_read < data_chunk_size);
    KALDI_WARN << "Read fewer bytes than specified in the header: "
               << num_bytes_read << " < " << data_chunk_size;    
  }
  
  if (data_chunk_size == 0)
    KALDI_ERR << "PCMData: empty file (no data)";
  
  uint32 num_samp = num_bytes_read / block_align;
  KALDI_LOG << "PCMData: there are " << num_samp << " samples";
  data_.Resize(num_channels, num_samp);
  for (uint32 i = 0; i < num_samp; i++) {
    for (uint32 j = 0; j < num_channels; j++) {
      switch (bits_per_sample) {
        case 8:
          data_(j, i) = *data_ptr;
          data_ptr++;
          break;
        case 16:
          {
            int16 k = *reinterpret_cast<uint16*>(data_ptr);
            if (swap)
              KALDI_SWAP2(k);
            data_(j, i) =  k;
            data_ptr += 2;
            break;
          }
        case 32:
          {
            int32 k = *reinterpret_cast<uint32*>(data_ptr);
            if (swap)
              KALDI_SWAP4(k);
            data_(j, i) =  k;
            data_ptr += 4;
            break;
          }
        default:
          KALDI_ERR << "bits per sample is " << bits_per_sample;  // already checked this.
      }
    }
  }
}


// Write 16-bit PCM.

// note: the WAVE chunk contains 2 subchunks.
//
// subchunk2size = data.NumRows() * data.NumCols() * 2.


void PCMData::Write(std::ostream &os) const {
  os << "RIFF";
  if (data_.NumRows() == 0)
    KALDI_ERR << "Error: attempting to write empty WAVE file";

  int32 num_chan = data_.NumRows(),
      num_samp = data_.NumCols(),
      bytes_per_samp = 2;

  int32 subchunk2size = (num_chan * num_samp * bytes_per_samp);
  int32 chunk_size = 36 + subchunk2size;
  WriteUint32(os, chunk_size);
  os << "WAVE";
  os << "fmt ";
  WriteUint32(os, 16);
  WriteUint16(os, 1);
  WriteUint16(os, num_chan);
  KALDI_ASSERT(samp_freq_ > 0);
  WriteUint32(os, static_cast<int32>(samp_freq_));
  WriteUint32(os, static_cast<int32>(samp_freq_) * num_chan * bytes_per_samp);
  WriteUint16(os, num_chan * bytes_per_samp);
  WriteUint16(os, 8 * bytes_per_samp);
  os << "data";
  WriteUint32(os, subchunk2size);

  const BaseFloat *data_ptr = data_.Data();
  int32 stride = data_.Stride();

  for (int32 i = 0; i < num_samp; i++) {
    for (int32 j = 0; j < num_chan; j++) {
      int32 elem = static_cast<int32>(data_ptr[j*stride + i]);
      int16 elem_16(elem);
      if (static_cast<int32>(elem_16) != elem)
        KALDI_ERR << "Wave file is out of range for 16-bit.";
#ifdef __BIG_ENDIAN__
      KALDI_SWAP2(elem_16);
#endif
      os.write(reinterpret_cast<char*>(&elem_16), 2);
    }
  }
  if (os.fail())
    KALDI_ERR << "Error writing wave data to stream.";
}


}  // end namespace kaldi
