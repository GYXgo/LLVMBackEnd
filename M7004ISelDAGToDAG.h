#include "llvm/CodeGen/SelectionDAGISel.h"
#include "MCTargetDesc/M7004MCTargetDesc.h"
#include "M7004.h"
#include "M7004Subtarget.h"
#include "M7004TargetMachine.h"

#define DEBUG_TYPE "m7004-isel"
#define PASS_NAME "M7004 DAG->DAG Pattern Instruction Selection"

namespace llvm{
//第一个子类
class M7004DAGToDAGISel : public SelectionDAGISel {
public:
  M7004DAGToDAGISel() = delete;
  explicit M7004DAGToDAGISel(M7004TargetMachine &TM)
      : SelectionDAGISel(TM), Subtarget(nullptr) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  // 地址选择函数
  bool SelectAddrFI(SDNode *Parent, SDValue AddrFI, SDValue &Base, SDValue &Offset);
  bool SelectRtoRMemAddrFI(SDNode *Parent, SDValue AddrFI, SDValue &Base,SDValue &Offset);
private:
  const M7004Subtarget *Subtarget;

  // 添加处理复杂分支条件的方法
  bool SelectComplexBrCond(SDNode *N);

#include "M7004GenDAGISel.inc"

  const M7004TargetMachine &getTargetMachine() {
    return static_cast<const M7004TargetMachine &>(TM);
  }

  void Select(SDNode *N) override;
};


class M7004DAGToDAGISelLegacy : public SelectionDAGISelLegacy {
public:
  static char ID;
  explicit M7004DAGToDAGISelLegacy(M7004TargetMachine &TM)
      : SelectionDAGISelLegacy(ID, std::make_unique<M7004DAGToDAGISel>(TM)) {}
};
}