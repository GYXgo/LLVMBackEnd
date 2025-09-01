#ifndef M7004InstrInfo_H
#define M7004InstrInfo_H

#include "llvm/MC/MCInstrInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "M7004GenInstrInfo.inc"

namespace llvm{
class M7004InstrInfo : public M7004GenInstrInfo
{
public:
	explicit M7004InstrInfo();

	void storeRegToStackSlot(MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI, Register SrcReg,
    bool IsKill, int FrameIndex,
    const TargetRegisterClass *RC,
    const TargetRegisterInfo *TRI,
    Register VReg) const override;

  void loadRegFromStackSlot(MachineBasicBlock &MBB,
     MachineBasicBlock::iterator MI, Register DestReg,
     int FrameIndex, const TargetRegisterClass *RC,
     const TargetRegisterInfo *TRI,
     Register VReg) const override;

  void copyPhysReg(MachineBasicBlock &MBB,
        MachineBasicBlock::iterator MI, const DebugLoc &DL,
        MCRegister DestReg, MCRegister SrcReg,
        bool KillSrc) const override;

          //对所有伪指令自动调用此函数，将伪指令展开为真实指令组合
  bool expandPostRAPseudo(MachineInstr &MI) const override;

};
}

#endif