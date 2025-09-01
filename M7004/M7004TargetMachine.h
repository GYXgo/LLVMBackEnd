#ifndef M7004TargetMachine_H
#define M7004TargetMachine_H

#include <optional>
#include "llvm/Target/TargetMachine.h"
#include "M7004Subtarget.h"
#include "MCTargetDesc/M7004MCTargetDesc.h"
#include "llvm/CodeGen/TargetPassConfig.h"

namespace llvm
{

class M7004TargetMachine : public LLVMTargetMachine
{
public:
	M7004TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
										 StringRef FS, const TargetOptions &Options,
										 std::optional<Reloc::Model> RM,
										 std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
										 bool JIT);

private:
  	std::unique_ptr<TargetLoweringObjectFile> TLOF;
	  M7004Subtarget Subtarget;
    
public:
	const M7004Subtarget *getSubtargetImpl(const Function &F) const override {
    return &Subtarget;
  }
  const M7004Subtarget *getSubtargetImpl() const { return &Subtarget; }


	TargetLoweringObjectFile *getObjFileLowering() const override
	{
		return TLOF.get();
	}

	TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
};

class M7004PassConfig : public TargetPassConfig {
public:
  M7004PassConfig(M7004TargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  M7004TargetMachine &getM7004TargetMachine() const {
    return getTM<M7004TargetMachine>();
  }

  const M7004Subtarget &getM7004Subtarget() const {
    return *getM7004TargetMachine().getSubtargetImpl();
  }
  bool addInstSelector() override; 

  // 禁用后续步骤
//时机：在寄存器分配（Register Allocation）之前调用。
//用途：可以在这里插入需要在寄存器分配前运行的 MachineFunctionPass，比如自定义的指令调整、伪指令展开等
  void addPreRegAlloc() override {}
//时机：在寄存器分配完成后调用。
//用途：可以在这里插入需要在寄存器分配后运行的 Pass，比如寄存器重命名、栈帧调整、寄存器冲突修复等。
  void addPostRegAlloc() override {}
//时机：在第二次调度（Sched2）之前调用。
//用途：可以插入在 MachineInstr 调度（如指令重排序）前需要运行的 Pass。一般用于指令流优化、延迟槽填充等。
  void addPreSched2() override {}
//时机：在最终 MachineInstrs 输出（Emit）之前调用。
//用途：可以插入在输出汇编/目标码前需要运行的 Pass，比如最后的伪指令消解、指令补丁、调试信息修正等。
  void addPreEmitPass() override {}
};
}

#endif
