#include "M7004Subtarget.h"

using namespace llvm;
#define DEBUG_TYPE "M7004-Subtarget"
#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "M7004GenSubtargetInfo.inc"


M7004Subtarget::M7004Subtarget(const Triple &TT, StringRef CPU, StringRef FS, const TargetMachine &TM)
    : M7004GenSubtargetInfo(TT, CPU, CPU, FS), RegInfo(*this),FrameLowering(*this), TLI(TM, *this) 
{

}

M7004Subtarget& M7004Subtarget::initializeSubtargetDependencies(const Triple &TT, StringRef CPU, StringRef FS, const TargetMachine &TM)
{

	if(CPU.empty()) // 此字符串见 M7004.td
	{
		CPU = "M7004";
	}
	ParseSubtargetFeatures(CPU, CPU, FS);
	return *this;
}
