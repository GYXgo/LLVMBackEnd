//
// Created by 86155 on 2025/4/16.
//

#ifndef LLVM_M7004ASMPRINTER_H
#define LLVM_M7004ASMPRINTER_H
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/MC/MCStreamer.h"
#include "M7004TargetMachine.h"

namespace llvm {
class M7004Subtarget;
class M7004AsmPrinter : public AsmPrinter
{
public:
  explicit M7004AsmPrinter(TargetMachine &TM,
                         std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer))
{
    Subtarget = static_cast<M7004TargetMachine &>(TM).getSubtargetImpl();
}

public:
  const M7004Subtarget *Subtarget;
  

public:
  StringRef getPassName() const override { return "M7004 Assembly Printer"; }

  virtual bool runOnMachineFunction(MachineFunction &MF) override;
  
  void emitInstruction(const MachineInstr *MI) override;
  
  
private:
  bool emitPseudoExpansionLowering(MCStreamer &OutStreamer,
                                   const MachineInstr *MI);
  MCOperand lowerSymbolOperand(const MachineOperand &MO) const;
  void lowerToMCInst(const MachineInstr *MI, MCInst &Out);
};
}

#endif // LLVM_M7004ASMPRINTER_H
