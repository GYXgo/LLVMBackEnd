// M7004ISelLowering.h - M7004 DAG Lowering (scaffolded from Fe)
#ifndef LLVM_LIB_TARGET_M7004_M7004ISELLOWERING_H
#define LLVM_LIB_TARGET_M7004_M7004ISELLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
class M7004Subtarget;
namespace M7004ISD {
enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  Call,
  RET_GLUE,
  SELECT,
  ADDframe 
};
}
class M7004TargetLowering : public TargetLowering {

public:
  explicit M7004TargetLowering(const TargetMachine &TM,
                               const M7004Subtarget &STI);
  const M7004Subtarget &getSubtarget() const { return Subtarget; }

private:
  const M7004Subtarget &Subtarget;

public:
  SDValue LowerCall(CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &DL, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
                      SelectionDAG &DAG) const override;

  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  const char *getTargetNodeName(unsigned Opcode) const override;

private:
  // 定义需要指令合法化的函数
  SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_M7004_M7004ISELLOWERING_H
