//
// Created by Copilot on 2025/6/25.
//

#include "M7004InstPrinter.h"
#include "M7004InstrInfo.h"
#include "M7004RegisterInfo.h"
#include "llvm/CodeGen/TargetOpcodes.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#define PRINT_ALIAS_INSTR
#include "M7004GenAsmWriter.inc"

std::unordered_map<unsigned int, std::string> llvm::M7004InstPrinter::dummyMap;

//构造函数
llvm::M7004InstPrinter::M7004InstPrinter(
    const MCAsmInfo &MAI, const MCInstrInfo &MII, const MCRegisterInfo &MRI,
    std::unordered_map<unsigned int, std::string> &Map)
    : MCInstPrinter(MAI, MII, MRI), MyMap(Map)
{
	
}

//寄存器打印函数
void M7004InstPrinter::printRegName(raw_ostream &OS, MCRegister Reg) const {
  if (Reg.id() >= 1024) {
    auto it = MyMap.find(Reg.id());
    if (it != MyMap.end())
      OS << it->second;
    else
      OS << '%' << (Reg.id());
  } else {
    OS << getRegisterName(Reg, M7004::NoRegAltName);
  }
}

//指令打印函数
void M7004InstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                 StringRef Annot, const MCSubtargetInfo &STI,
                                 raw_ostream &O) {
   if(MI->getOpcode() == M7004::PseudoRET )
   {
      O<<"\t.RET";
      return;
   }
   if(MI->getOpcode() == M7004::SELECT_PSEUDO)
   {
      // 处理SELECT_PSEUDO指令
      O << "\tSELECT\t";
      printOperand(MI, 0, O); // 假设第一个操作数是目标寄存器
      O << ", ";
      printOperand(MI, 1, O); // 假设第二个操作数是条件寄存器
      O << ", ";
      printOperand(MI, 2, O); // 假设第三个操作数是源寄存器
      O << ", ";
      printOperand(MI, 3, O); // 假设第四个操作数是源寄存器
      return;
   }  
   if(MI->getOpcode() == TargetOpcode::PHI)
   {
      // 处理PHI指令
      O << "\tPHI\t";
      for (unsigned i = 0, e = MI->getNumOperands(); i < e; ++i) {
        if (i > 0)
          O << ", ";
        printOperand(MI, i, O);
      }
      return;
   }
   if(MI->getOpcode() == TargetOpcode::COPY)        
   {
      // 处理COPY指令
      O << "\tCOPY\t";
      printOperand(MI, 1, O); // 假设第二个操作数是目标寄存器
      O << ", ";
      printOperand(MI, 0, O); // 假设第一个操作数是源寄存器
      return;
   }
  if (MI->getOpcode() == M7004::CALL || MI->getOpcode() == M7004::CALL_VOID) {
    O << "\tCALL ";
    unsigned CalleeIdx = 0;
    unsigned ParamStart = 1;
    unsigned ParamEnd = MI->getNumOperands();

    if(MI->getOpcode() == M7004::CALL)
    {
      // 打印 Callee
      printOperand(MI, ParamStart, O);
      // 获取参数个数（最后一个操作数）
      unsigned NumArgs = MI->getOperand(ParamStart+1).getImm();
      if(NumArgs!=0)  O << ",";
      // 打印参数模板
      for (unsigned i = 1; i <= NumArgs; ++i) {
          O << "_%_";
          O << "P" << i;
      }
      // 打印返回值（有返回值时）
      O << ",";
      //printOperand(MI, CalleeIdx, O);
      O << "RETR";
    }
    else if(MI->getOpcode() == M7004::CALL_VOID)
    {
      // 打印 Callee
      printOperand(MI, CalleeIdx, O);
      // 获取参数个数（最后一个操作数）
      unsigned NumArgs = MI->getOperand(CalleeIdx+1).getImm();
      if(NumArgs!=0)  O << ",";
      // 打印参数
      for (unsigned i = 1; i <= NumArgs; ++i) {
          O << "_%_";
          O << "P" << i;
      }
    }
    //O << "\n";
    return;
  }        
  if (!printAliasInstr(MI, Address, O))
    printInstruction(MI, Address, O);
  printAnnotation(O, Annot);
}

//打印操作数
void M7004InstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                  raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNo);
  if (MO.isReg()) {
    printRegName(O, MO.getReg());
    return;
  }
  if (MO.isImm()) {
    printImmediate(MI, OpNo, O);
    return;
  }
  if (MO.isSFPImm()) {
    // 处理单精度浮点立即数
    printFPImmOperand(MI, OpNo, O);
    return;
  }
  assert(MO.isExpr() && "Unknown operand kind in printOperand");
  MO.getExpr()->print(O, &MAI);
}

//打印立即数
void M7004InstPrinter::printImmediate(const MCInst *MI, unsigned opNum,
                                    raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(opNum);
  if (MO.isImm())
    O << MO.getImm();
  else if (MO.isExpr()) {
    MO.getExpr()->print(O, &MAI);
  } else
    llvm_unreachable("Unknown immediate kind");
}

void M7004InstPrinter::printFPImmOperand(const MCInst *MI, unsigned OpNo,
                                         raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNo);
  
  if (MO.isSFPImm()) {
    // 处理单精度浮点立即数
    uint32_t IntBits = static_cast<uint32_t>(MO.getSFPImm());
    float FPVal;
    memcpy(&FPVal, &IntBits, sizeof(float));
    O << format("%.6f", FPVal);
  } else {
    llvm_unreachable("Unknown floating point immediate operand kind");
  }
}

//打印分支操作数
void M7004InstPrinter::printBranchOperand(const MCInst *MI, unsigned OpNo,
                                       raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isExpr())
    Op.getExpr()->print(O, &MAI);
  else
    O << Op.getImm();
}

//获取物理寄存器名字
const char *M7004InstPrinter::getRegisterName(MCRegister Reg){
  return getRegisterName(Reg, M7004::NoRegAltName);
}

//打印mem内存地址
void M7004InstPrinter::printMemOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O) {
  /// 先打印立即数 12(sp)
  printOperand(MI, OpNo + 1, O);
  O << "(";
  printOperand(MI, OpNo, O);
  O << ")";
}

//打印RtoRmem内存地址
void M7004InstPrinter::printRtoRmemOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O)
{
  // 打印基址寄存器
  printOperand(MI, OpNo, O);
  //O << ",";
  // 打印偏移量寄存器
  //printOperand(MI, OpNo + 1, O);
}

void M7004InstPrinter::printBaseMemOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O)
{
  // 打印基址寄存器
  printOperand(MI, OpNo, O);
}

