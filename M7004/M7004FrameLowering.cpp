//
// Created by Copilot on 2025/6/25.
//

#include "M7004FrameLowering.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "MCTargetDesc/M7004MCTargetDesc.h"
#include "M7004.h"
#include "M7004Subtarget.h"

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
using namespace llvm;

// 计算栈帧大小
uint64_t M7004FrameLowering::computeStateSize(MachineFunction &MF) const {
  uint64_t STACKSIZE = MF.getFrameInfo().getStackSize();
  if (getStackAlignment() > 0) {
    STACKSIZE = ROUND_UP(STACKSIZE, getStackAlignment());
  }
  return STACKSIZE;
}

// 前序(现场保护)
void M7004FrameLowering::emitPrologue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  // 定位基本块中第一条指令（MBBI），用于插入新的指令。
  MachineBasicBlock::iterator MBBI = MBB.begin();
  const TargetInstrInfo &TII = *STI.getInstrInfo();
  int STACKSIZE = computeStateSize(MF);
  if (STACKSIZE == 0) {
    return;
  }
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  BuildMI(MBB, MBBI, DL, TII.get(M7004::SADD32_imm), M7004::AR15)
      .addReg(M7004::AR15)
      .addImm(-STACKSIZE)
      .setMIFlag(MachineInstr::FrameSetup);
}

// 尾声（现场恢复）
void M7004FrameLowering::emitEpilogue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  const TargetInstrInfo &TII = *STI.getInstrInfo();
  int STACKSIZE = computeStateSize(MF);
  if (STACKSIZE == 0) {
    return;
  }
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  BuildMI(MBB, MBBI, DL, TII.get(M7004::SADD32_imm), M7004::AR15)
      .addReg(M7004::AR15)
      .addImm(STACKSIZE)
      .setMIFlag(MachineInstr::FrameDestroy);
}

//识别需要被调用者保存的寄存器​​（LS自行处理？）
void M7004FrameLowering::determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs, RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  if (MF.getFrameInfo().hasCalls()) {
    //SavedRegs.set(M7004::RS1);
  }
}

bool M7004FrameLowering::hasFP(const MachineFunction &MF) const
{
  return false;
}
