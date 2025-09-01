#ifndef M7004SUBTARGET_H
#define M7004SUBTARGET_H

#include "M7004InstrInfo.h"
#include "M7004RegisterInfo.h"
#include "M7004FrameLowering.h"
#include "M7004ISelLowering.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"

#define GET_SUBTARGETINFO_HEADER
#include "M7004GenSubtargetInfo.inc"

namespace llvm{

class M7004Subtarget : public M7004GenSubtargetInfo
{
private:
	M7004InstrInfo InstrInfo;
	M7004RegisterInfo RegInfo;
  M7004FrameLowering FrameLowering;
  M7004TargetLowering TLI;

public:
	M7004Subtarget(const Triple &TT, StringRef CPU, StringRef Features, const TargetMachine &TM);
	M7004Subtarget& initializeSubtargetDependencies(const Triple &TT, StringRef CPU, StringRef FS, const TargetMachine &TM);


	const M7004InstrInfo *getInstrInfo() const override { return &InstrInfo; }
	const M7004RegisterInfo *getRegisterInfo() const override { return &RegInfo; }

  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);
  const M7004FrameLowering *getFrameLowering() const override { return &FrameLowering; }
  const M7004TargetLowering *getTargetLowering() const override { return &TLI; }
};

}

#endif // M7004SUBTARGET_H
