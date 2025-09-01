#include "M7004TargetMachine.h"
#include "TargetInfo/M7004TargetInfo.h"
#include "M7004.h"
#include "M7004ISelPrinter.h" 
#include "llvm/MC/TargetRegistry.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#define DEBUG_TYPE "M7004"

using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeM7004Target()
{
	// extern Target TheFooTarget;
	RegisterTargetMachine<M7004TargetMachine> X(getM7004Target());
auto *PR = PassRegistry::getPassRegistry();
	initializeM7004DAGToDAGISelLegacyPass(*PR);
	initializeM7004ISelPrinterPass(*PR);
}

static StringRef computeDataLayout(const Triple &TT, const TargetOptions &Options)
{
	assert(TT.isArch32Bit() && "only 32 bit are currently supported");
	//StringRef ABIName = Options.MCOptions.getABIName();
/*	if (TT.isArch64Bit()) {
		// if (ABIName == "lp64e")
		return "e-m:e-p:64:64-i64:32-n32-S128";
		// return "e-m:e-p:64:64-i64:64-i128:128-n32:64-S128";
	}

	// return "e-m:e-p:32:32-i64:64-n32-S128";
*/
	return "e-m:e-p:32:32-i64:64-n32-S128";
}

static Reloc::Model getEffectiveRelocModel(const Triple &TT, std::optional<Reloc::Model> RM)
{
	return RM.value_or(Reloc::Static);
	// RM是标准库的optional结构对象，在此结构下，value_or功能为如果传了RM参数进来，就用RM的值，否则就用括号里的值。
}

M7004TargetMachine::M7004TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
																			 StringRef FS, const TargetOptions &Options,
																			 std::optional<Reloc::Model> RM,
																			 std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
																			 bool JIT)
	: LLVMTargetMachine(T, computeDataLayout(TT, Options), TT, CPU, FS, Options,
											getEffectiveRelocModel(TT, RM),
											getEffectiveCodeModel(CM, CodeModel::Small), OL),
		TLOF(std::make_unique<TargetLoweringObjectFileELF>()),Subtarget(TT, CPU, FS, *this) // 仅得到汇编代码不必子类化，这里用此父类名。
{
	initAsmInfo(); // 可能需要生成汇编指令，所以先调用这个函数进行初始化。
}

TargetPassConfig *M7004TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new M7004PassConfig(*this, PM);
}

bool M7004PassConfig::addInstSelector()
{
  // Install an instruction selector.
  addPass(createM7004ISelDag(getM7004TargetMachine()));
  addPass(createM7004ISelPrinter());

  return false;
}