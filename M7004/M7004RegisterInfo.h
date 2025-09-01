#ifndef M7004REGISTERINFO_H
#define M7004REGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"
#define GET_REGINFO_HEADER
#include "M7004GenRegisterInfo.inc"

namespace llvm{
class M7004Subtarget;
class M7004RegisterInfo : public M7004GenRegisterInfo
{
public:
	M7004RegisterInfo(const M7004Subtarget &STI);

private:
  const M7004Subtarget &STI;

public:
	const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
	const uint32_t *getCallPreservedMask(const MachineFunction &MF, CallingConv::ID) const override;
	BitVector getReservedRegs(const MachineFunction &MF) const override;
	bool eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj, unsigned FIOperandNum, RegScavenger *RS = nullptr) const override;
	Register getFrameRegister(const MachineFunction &MF) const override;

};
}

#endif // M7004REGISTERINFO_H
