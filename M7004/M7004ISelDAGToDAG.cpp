#include "M7004ISelDAGToDAG.h"

using namespace llvm;

bool M7004DAGToDAGISel::runOnMachineFunction(MachineFunction &MF) {
  Subtarget = &MF.getSubtarget<M7004Subtarget>();
  return SelectionDAGISel::runOnMachineFunction(MF);
}

void M7004DAGToDAGISel::Select(SDNode *Node)
{
  SDLoc DL(Node);
  switch (Node->getOpcode()) {
  case ISD::BRCOND: {
    SDValue Cond = Node->getOperand(1);
    // 检查第二个操作数是否为常量0
    if (ConstantSDNode *C = dyn_cast<ConstantSDNode>(Cond)) {
      if (C->getZExtValue() == 0) {
        // 直接删除该节点
        SDValue Chain = Node->getOperand(0);
        ReplaceNode(Node, Chain.getNode());
        return;
      } else {
        // 非0，直接生成跳转指令
        SDValue Chain = Node->getOperand(0);
        SDValue Dest = Node->getOperand(2);
        // 生成你的跳转指令，比如 JUMP
        MachineSDNode *J =
            CurDAG->getMachineNode(M7004::SBR_imm, SDLoc(Node), MVT::Other, Chain, Dest);
        ReplaceNode(Node, J);
        return;
      }
    }
    break;
  }
  case ISD::SELECT: {
    SDValue Cond = Node->getOperand(0);
    SDValue TrueVal = Node->getOperand(1);
    SDValue FalseVal = Node->getOperand(2);
    EVT VT = Node->getValueType(0);

    // 生成 SELECT_PSEUDO，输入为 Cond, TrueVal,
    // FalseVal，输出为同一个虚拟寄存器
    SDNode *SelectNode = CurDAG->getMachineNode(M7004::SELECT_PSEUDO, DL, VT,
                                                Cond, TrueVal, FalseVal);

    // 用虚拟寄存器替换SELECT节点
    ReplaceNode(Node, SelectNode);
    return;
  }
  case ISD::FrameIndex: {
    assert(Node->getValueType(0) == MVT::i32);
    SDLoc dl(Node);
    int FI = cast<FrameIndexSDNode>(Node)->getIndex();
    SDValue TFI = CurDAG->getTargetFrameIndex(FI, MVT::i32);
    if (Node->hasOneUse()) {
      CurDAG->SelectNodeTo(Node, M7004::ADDframe, MVT::i32, TFI,
                           CurDAG->getTargetConstant(0, dl, MVT::i32));
      return;
    }
    ReplaceNode(Node, CurDAG->getMachineNode(
                          M7004::ADDframe, dl, MVT::i32, TFI,
                          CurDAG->getTargetConstant(0, dl, MVT::i32)));
    return;
  }
  case M7004ISD::Call: {
    SDLoc dl(Node);
    std::vector<SDValue> Ops;
    for (unsigned i = 1; i < Node->getNumOperands(); ++i) {
      Ops.push_back(Node->getOperand(i));
    }
    unsigned NumVals = Node->getNumValues();
    SDNode *CallNode = nullptr;
    if (NumVals == 3) {
      EVT RetVT = Node->getValueType(0);
      CallNode = CurDAG->getMachineNode(M7004::CALL, dl, RetVT, MVT::Other,
                                        MVT::Glue, Ops);
    } else if (NumVals == 2) {
      CallNode = CurDAG->getMachineNode(M7004::CALL_VOID, dl, MVT::Other,
                                        MVT::Glue, Ops);
    } else {
      llvm_unreachable("Unexpected number of values for PeISD::Call!");
    }
    ReplaceNode(Node, CallNode);
    return;
  }

  default:
    break;
  }

  LLVM_DEBUG(dbgs() << "Selecting: "; Node->dump(CurDAG); dbgs() << '\n');
  SelectCode(Node);// 若上面的分支中没有匹配上， 则此函数用td的匹配规则匹配。
}

// td文件中声明需要用到的函数.
bool M7004DAGToDAGISel::SelectAddrFI(SDNode *Parent, SDValue AddrFI, SDValue &Base, SDValue &Offset) {
  /// FrameIndex -> TargetFrameIndex
  if(FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(AddrFI)) {
    Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), AddrFI.getValueType());
    Offset = CurDAG->getTargetConstant(0, SDLoc(AddrFI), AddrFI.getValueType());
    return true;
  }
  if (CurDAG->isBaseWithConstantOffset(AddrFI)) {
    ConstantSDNode *CN = dyn_cast<ConstantSDNode>(AddrFI.getOperand(1));

    if (FrameIndexSDNode *FS = dyn_cast<FrameIndexSDNode>(AddrFI.getOperand(0))) {
      Base = CurDAG->getTargetFrameIndex(FS->getIndex(), AddrFI.getValueType());
    }else {
      Base = AddrFI.getOperand(0);
    }
    Offset = CurDAG->getTargetConstant(CN->getZExtValue(), SDLoc(AddrFI), AddrFI.getValueType());
    return true;
  }
  return false;
}

// td文件中声明需要用到的函数.
bool M7004DAGToDAGISel::SelectRtoRMemAddrFI(SDNode *Parent, SDValue AddrFI, SDValue &Base, SDValue &Offset)
{
  if (AddrFI.getOpcode() == ISD::ADD) {
    SDValue LHS = AddrFI.getOperand(0);
    SDValue RHS = AddrFI.getOperand(1);
    // FrameIndex + 任意i32类型（如shl、寄存器、add等）
    if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(LHS.getNode())) {
      Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), LHS.getValueType());
      Offset = RHS;
      return true;
    }
    if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(RHS.getNode())) {
      Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), RHS.getValueType());
      Offset = LHS;
      return true;
    }
  }
  return false;
}

char M7004DAGToDAGISelLegacy::ID;

INITIALIZE_PASS(M7004DAGToDAGISelLegacy, DEBUG_TYPE, PASS_NAME, false, false)//注册M7004DAGToDAGISelLegacy给llvm
FunctionPass *llvm::createM7004ISelDag(M7004TargetMachine &TM)
{ //创建一个pass, 实现指令选择.
  return new M7004DAGToDAGISelLegacy(TM);
}