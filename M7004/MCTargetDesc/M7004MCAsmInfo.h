#ifndef M7004MCAsmInfo_H
#define M7004MCAsmInfo_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm
{
	class Triple;
	
class M7004MCAsmInfo : public MCAsmInfoELF
{
	public:
	explicit M7004MCAsmInfo(const Triple & TT);

};
}

#endif