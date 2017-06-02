#ifndef CPU_INIT_H
#define CPU_INIT_H

#include "mylib/option.h"
#include "mylib/ubm.h"
#include "mylib/gettime.h"

class InitSRE
{
	public:
		InitSRE() {}
		bool ReadUBMFile(std::string final_ubm, std::string final_ie, std::string gmm_ubm);
		
		~InitSRE() {}
		SREUBM *ubm;
};


#endif
