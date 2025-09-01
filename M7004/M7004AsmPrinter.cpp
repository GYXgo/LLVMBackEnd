//
// Created by Copilot on 2025/6/25.
//

#include "M7004AsmPrinter.h"
#include "MCTargetDesc/M7004MCExpr.h"
#include "MCTargetDesc/M7004MCTargetDesc.h"
#include "TargetInfo/M7004TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#define PRINT_ALIAS_INSTR
#include "M7004GenMCPseudoLowering.inc"


bool M7004AsmPrinter::runOnMachineFunction(MachineFunction &MF)
{
  AsmPrinter::runOnMachineFunction(MF);
  return true;
}

//输出一条机器指令。
void M7004AsmPrinter::emitInstruction(const MachineInstr *MI)
{
  if (emitPseudoExpansionLowering(*OutStreamer, MI)) {
    return;
  }
  MCInst TmpInst;
  lowerToMCInst(MI, TmpInst);
  EmitToStreamer(*OutStreamer, TmpInst);
}

//将一条 MachineInstr 转换为 MCInst。
void M7004AsmPrinter::lowerToMCInst(const MachineInstr *MI, MCInst &Out) {
  Out.setOpcode(MI->getOpcode());
  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp;
    switch (MO.getType()) {
    case MachineOperand::MO_Register: {
      MCOp = MCOperand::createReg(MO.getReg());
      break;
    }
    case MachineOperand::MO_Immediate: {
      MCOp = MCOperand::createImm(MO.getImm());
      break;
    }
    case MachineOperand::MO_FPImmediate: {
      const APFloat &APF = MO.getFPImm()->getValueAPF();
      MCOp = MCOperand::createSFPImm(APF.bitcastToAPInt().getZExtValue());
      break;
    }
    case MachineOperand::MO_MachineBasicBlock: 
    case MachineOperand::MO_GlobalAddress:
    {
      MCOp = lowerSymbolOperand(MO);
      break;
    }
    case MachineOperand::MO_RegisterMask: {
      /// Ignore
      break;
    }
    default:
      llvm::errs() << "Unknown operand type: " << MO.getType() << "\n";
      llvm::errs() << "Instruction: ";
      MI->print(llvm::errs());
      llvm::errs() << "\n";
      llvm_unreachable("unknown operand type");
    }
    Out.addOperand(MCOp);
  }
}

//将符号类操作数（如全局变量、外部符号、基本块）转换为 MCOperand。
MCOperand M7004AsmPrinter::lowerSymbolOperand(const MachineOperand &MO) const {
  const MCSymbol *Symbol;
  M7004MCExpr::M7004ExprKind TargetKind = M7004MCExpr::NONE;

  switch (MO.getType()) {
  case MachineOperand::MO_GlobalAddress:
    Symbol = getSymbol(MO.getGlobal());
    break;
  case MachineOperand::MO_ExternalSymbol:
    Symbol = GetExternalSymbolSymbol(MO.getSymbolName());
    break;
  case MachineOperand::MO_MachineBasicBlock:
    Symbol = MO.getMBB()->getSymbol();
    break;
  default:
    llvm_unreachable_internal("unknown operand type");
  }

  const MCExpr *Expr =
      MCSymbolRefExpr::create(Symbol, MCSymbolRefExpr::VK_None, OutContext);
  Expr = new M7004MCExpr(TargetKind, Expr);

  return MCOperand::createExpr(Expr);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeM7004AsmPrinter()
{
  RegisterAsmPrinter<M7004AsmPrinter> X(getM7004Target());
}
