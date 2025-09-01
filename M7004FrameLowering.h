// M7004FrameLowering.h - M7004 Frame Lowering (scaffolded from Fe)
#ifndef LLVM_LIB_TARGET_M7004_M7004FRAMELOWERING_H
#define LLVM_LIB_TARGET_M7004_M7004FRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {
class M7004Subtarget;
class M7004FrameLowering : public TargetFrameLowering {
  
public:
  explicit M7004FrameLowering(const M7004Subtarget &STI)
      : TargetFrameLowering(StackGrowsDown, Align(4), 0, Align(4)), STI(STI) {}

private:
  const M7004Subtarget &STI;

public:
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs, RegScavenger *RS) const override;
protected:
  bool hasFP(const MachineFunction &MF) const override;

private:
  uint64_t computeStateSize(MachineFunction &MF) const;
    
};
}

#endif // LLVM_LIB_TARGET_M7004_M7004FRAMELOWERING_H
