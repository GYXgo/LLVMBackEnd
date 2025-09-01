#include "llvm/MC/TargetRegistry.h"
#include "TargetInfo/M7004TargetInfo.h"
#include "M7004MCAsmInfo.h"
#include "M7004InstrInfo.h"
#include "M7004RegisterInfo.h"
#include "M7004Subtarget.h"
#include "M7004MCTargetDesc.h"
#include "M7004InstPrinter.h"

#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;
#define GET_INSTRINFO_MC_DESC
#include "M7004GenInstrInfo.inc"

#include <cstdint>
#define GET_REGINFO_MC_DESC
#include "M7004GenRegisterInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "M7004GenSubtargetInfo.inc"

namespace {
//创建并返回 M7004 目标的汇编信息对象（如注释风格、指令分隔符、数据对齐等）。
MCAsmInfo * createM7004MCAsmInfo(const MCRegisterInfo &MRI, const Triple &TT, const MCTargetOptions &options)
{
	MCAsmInfo *X = new M7004MCAsmInfo(TT);
	return X;
}

//创建并初始化 M7004 的寄存器信息对象
MCRegisterInfo * createM7004MCRegisterInfo(const Triple &TT)
{
	MCRegisterInfo *X = new MCRegisterInfo();
	InitM7004MCRegisterInfo(X, M7004::R10);
	return X;
}

//创建并初始化 M7004 的指令信息对象。
MCInstrInfo * createM7004MCInstrInfo()
{
	MCInstrInfo *X = new MCInstrInfo();
	InitM7004MCInstrInfo(X);// ? , M7004::RA
	return X;
}

//创建并初始化 M7004 的子目标信息对象（如不同 CPU 型号、特性组合）。
MCSubtargetInfo * createM7004MCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef features)
{
	if(CPU.empty()) // 此字符串见 M7004.td
	{
		CPU = "M7004";
	}
	return createM7004MCSubtargetInfoImpl(TT, CPU, CPU,features);
}

//创建 M7004 的指令打印器对象。
MCInstPrinter *createM7004MCInstPrinter(const Triple &T, unsigned SyntaxVariant,
                                      const MCAsmInfo &MAI,
                                      const MCInstrInfo &MII,
                                      const MCRegisterInfo &MRI) 
{
  return new M7004InstPrinter(MAI, MII, MRI);
}
}

/*
 * llc的目标后端都必须实现各自的void LLVMInitializeXXXTargetMC()函数。否则，编译目标后端MyRISCV时会报错undefined reference to 'LLVMInitializeM7004TargetMC'。
 * */
//注册信息 和添加target类似 固定操作
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeM7004TargetMC()
{
	for(Target *T : {& getM7004Target()})
	{
		TargetRegistry::RegisterMCAsmInfo(*T, createM7004MCAsmInfo);
		TargetRegistry::RegisterMCRegInfo(*T, createM7004MCRegisterInfo);
		TargetRegistry::RegisterMCInstrInfo(*T, createM7004MCInstrInfo);
		TargetRegistry::RegisterMCSubtargetInfo(*T, createM7004MCSubtargetInfo);
		TargetRegistry::RegisterMCInstPrinter(*T, createM7004MCInstPrinter);
		// TargetRegistry::RegisterMCInstPrinter(*T, createM7004MCInstPrinter);
	}
 }
