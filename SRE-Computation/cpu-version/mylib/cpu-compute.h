#ifndef CPU_COMPUTE_H
#define CPU_COMPUTE_H

#include "mylib/sre.h"
#include "mylib/gettime.h"
#include "mylib/cpu-init.h"
#include "mylib/tool.h"

class CpuCompute
{
	public:
		CpuCompute(){}
		~CpuCompute(){}

		bool Compute(InitSRE *ubm, std::string filename, std::string ivector_path, int valid_frames);
};


#endif
