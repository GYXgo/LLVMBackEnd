#include "M7004RegisterInfo.h"
#include "MCTargetDesc/M7004MCTargetDesc.h"
#include "M7004Subtarget.h"
#include "M7004.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "M7004GenRegisterInfo.inc"
#define DEBUG_TYPE "m7004 register info"

M7004RegisterInfo::M7004RegisterInfo(const M7004Subtarget &STI): M7004GenRegisterInfo(M7004::R62),STI(STI)// 返回地址寄存器
 {

 }

const MCPhysReg *M7004RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const
{
  static const MCPhysReg CalleeSavedRegs[] = {M7004::AR15};
  return CalleeSavedRegs;
}

const uint32_t *M7004RegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                     CallingConv::ID) const {
  return M7004_CSR_RegMask;
}

BitVector M7004RegisterInfo::getReservedRegs(const MachineFunction &MF) const
{
	BitVector reserved(getNumRegs());

	reserved.set(M7004::AR15);
	reserved.set(M7004::R6);
	reserved.set(M7004::R7);

	return reserved;
}

bool M7004RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                            int SPAdj, unsigned FIOperandNum,
                                            RegScavenger *RS) const {
  MachineInstr &MI = *II;
  LLVM_DEBUG(errs() << MI);
  // 找到FrameIndex操作数
  unsigned I = 0;
  while (!MI.getOperand(I).isFI()) {
    ++I;
    assert(I < MI.getNumOperands());
  }
  int FI = MI.getOperand(I).getIndex();

  // 获取栈帧偏移
  MachineFunction &MF = *MI.getParent()->getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  int64_t Offset = MFI.getObjectOffset(FI);
  uint64_t StackSize =
      ROUND_UP(MFI.getStackSize(), STI.getFrameLowering()->getStackAlignment());
  Offset += static_cast<int64_t>(StackSize);

  // 专门处理 ADDframe 伪指令
  if (MI.getOpcode() == M7004::ADDframe) {
    const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
    Register BaseReg = M7004::AR15; // 假设RS0为帧指针
    Register DstReg = MI.getOperand(0).getReg();

    // 1. MOV BaseReg, DstReg
    BuildMI(*MI.getParent(), MI, MI.getDebugLoc(), TII.get(M7004::MOV_A_R),
            DstReg)
        .addReg(BaseReg);

    // 2. 如果有偏移，DstReg = DstReg + Offset
    if (Offset != 0) {
      BuildMI(*MI.getParent(), MI, MI.getDebugLoc(), TII.get(M7004::SADD32_R),
              DstReg)
          .addReg(DstReg)
          .addImm(Offset);
    }

    MI.eraseFromParent();
    return true;
  }
  unsigned NumOps = MI.getNumOperands();
  if (NumOps > I + 1) 
  {
    MachineOperand &OpNext = MI.getOperand(I + 1);
    if (OpNext.isImm()) {
      Offset += OpNext.getImm();
      MI.getOperand(I).ChangeToRegister(M7004::AR15, false);
      MI.getOperand(I + 1).ChangeToImmediate(Offset);
    } 
    else if (OpNext.isReg()) {
      Register IndexReg = MI.getOperand(I + 1).getReg();
      // Register AddrReg = Pe::RS1; // 默认使用RS0作为地址寄存器
      // MachineRegisterInfo &MRI = MF.getRegInfo();
      // for (unsigned Reg = Pe::RS3; Reg <= Pe::RS31; ++Reg) {
      //   if (!MRI.isPhysRegUsed(Reg) && Reg != Pe::RS10 && Reg != Pe::RS11) {
      //     // Reg 是当前未被分配的物理寄存器
      //     AddrReg = Reg;
      //     llvm::errs() << "printRegName called with RegNo=" << Reg << "\n";
      //     break;
      //   }
      // }

      // 分配一个虚拟寄存器
      MachineRegisterInfo &MRI = MF.getRegInfo();
      Register AddrReg = MRI.createVirtualRegister(&M7004::GP32SRRegClass);

      // 1. BaseReg = RS0 + Offset
      BuildMI(*MI.getParent(), MI, MI.getDebugLoc(),
              MF.getSubtarget().getInstrInfo()->get(M7004::MOV_A_R), AddrReg)
          .addImm(Offset);

      // 2. AddrReg = AddrReg + IndexReg
      BuildMI(*MI.getParent(), MI, MI.getDebugLoc(),
              MF.getSubtarget().getInstrInfo()->get(M7004::SADD32_R), AddrReg)
          .addReg(AddrReg)
          .addReg(IndexReg);

      MI.getOperand(I).ChangeToRegister(AddrReg, false);
      // 这里可以选择保留第二个操作数（如设为0），或者删掉（如果硬件不需要）
      MI.getOperand(I + 1).ChangeToImmediate(0);
      // MI.removeOperand(I + 1);
    } 
    else 
    {
      llvm_unreachable("Unsupported operand after FrameIndex!");
    }
  } 
  else 
  {
    // 只有一个操作数，直接用帧指针
    MI.getOperand(I).ChangeToRegister(M7004::AR15, false);
    // 如果需要，可以加偏移
  }
  return true;
}

Register M7004RegisterInfo::getFrameRegister(const MachineFunction &MF) const
{
	return M7004::AR15;
}

