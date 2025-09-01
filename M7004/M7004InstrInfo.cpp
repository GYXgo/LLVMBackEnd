#include "M7004InstrInfo.h"
#include "MCTargetDesc/M7004MCTargetDesc.h"
using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "M7004GenInstrInfo.inc"

M7004InstrInfo::M7004InstrInfo() : M7004GenInstrInfo()
{

}

//将寄存器值保存到栈帧
void M7004InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI, Register SrcReg,
    bool IsKill, int FrameIndex,
    const TargetRegisterClass *RC,
    const TargetRegisterInfo *TRI,
    Register VReg) const 
{
    DebugLoc DL;
    BuildMI(MBB, MI, DL, get(M7004::STORE_imm))
        .addReg(SrcReg,getKillRegState(IsKill))
        .addFrameIndex(FrameIndex)
        .addImm(0); 

}

//从栈帧加载到寄存器
void M7004InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
     MachineBasicBlock::iterator MI, Register DestReg,
     int FrameIndex, const TargetRegisterClass *RC,
     const TargetRegisterInfo *TRI,
     Register VReg) const
{
    DebugLoc DL;
    BuildMI(MBB, MI, DL, get(M7004::LOAD_imm),DestReg)
        .addFrameIndex(FrameIndex)
        .addImm(0); 

}

//将物理寄存器的值复制到另一个物理寄存器
void M7004InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI,
    const DebugLoc &DL, MCRegister DestReg,
    MCRegister SrcReg, bool KillSrc) const 
{
    MachineInstrBuilder MIB = BuildMI(MBB, MI, DL, get(M7004::MOV_A_R));
    MIB.addReg(DestReg, RegState::Define);
    MIB.addReg(SrcReg, getKillRegState(KillSrc));
}

bool M7004InstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  case M7004::SELECT_PSEUDO: {
    MachineBasicBlock &MBB = *MI.getParent();
    const DebugLoc &DL = MI.getDebugLoc();
    Register Dst = MI.getOperand(0).getReg();
    Register Cond = MI.getOperand(1).getReg();
    Register TVal = MI.getOperand(2).getReg();
    Register FVal = MI.getOperand(3).getReg();

    // 1. 条件成立时写Dst
    BuildMI(MBB, MI, DL, get(M7004::MOV_A_Cond), Dst).addReg(Cond).addReg(TVal);

    // 2. 条件不成立时写Dst
    BuildMI(MBB, MI, DL, get(M7004::MOV_A_NotCond), Dst)
        .addReg(Cond)
        .addReg(FVal);

    MI.eraseFromParent();
    return true;
  }
  default:
    // 其他伪指令不处理
    break;
  }
  return false;
}