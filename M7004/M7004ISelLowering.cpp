//
// Created by 86155 on 2025/4/16.
//

#include "M7004ISelLowering.h"
#include "M7004Subtarget.h"
#include "M7004TargetMachine.h"
#include "MCTargetDesc/M7004MCTargetDesc.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"

using namespace llvm;
#include "M7004GenCallingConv.inc"

M7004TargetLowering::M7004TargetLowering(const TargetMachine &TM,
                                         const M7004Subtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {
  /// 注册RegiserClass
  /// 还要处理合法化（类型和操作）
  addRegisterClass(MVT::i32, &M7004::GP32SRRegClass);
  addRegisterClass(MVT::f32, &M7004::FP32SRRegClass);
  addRegisterClass(MVT::v16i32, &M7004::GP32VRRegClass); // 添加向量寄存
  addRegisterClass(MVT::v16f32, &M7004::FP32VRRegClass); // 添加向量寄存
  // 添加浮点常量支持
  setOperationAction(ISD::ConstantFP, MVT::f32, Legal);
  computeRegisterProperties(STI.getRegisterInfo());

  // 将 select_cc 展开为分支指令
  setOperationAction(ISD::SELECT_CC, MVT::i32, Custom);
  setOperationAction(ISD::SELECT, MVT::i32, Legal);

  setOperationAction(ISD::BR_CC, MVT::i32, Expand);
  setOperationAction(ISD::SETCC, MVT::i32, Legal);

  // 设置内存操作
  setOperationAction(ISD::LOAD, MVT::i32, Legal);
  setOperationAction(ISD::STORE, MVT::i32, Legal);

}
// 其他的合法化操作. 目前还没有做.
SDValue M7004TargetLowering::LowerOperation(SDValue Op,
                                            SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::SELECT_CC:
    return LowerSELECT_CC(Op, DAG);
  // case ISD::SELECT:
  //   return LowerSELECT(Op, DAG);
  default:
    llvm::llvm_unreachable_internal("unknown op");
  }
  return SDValue();
}

SDValue M7004TargetLowering::LowerCall(CallLoweringInfo &CLI,
                                       SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;

  MachineFunction &MF = DAG.getMachineFunction();
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeCallOperands(Outs, CC_M7004);

  SmallVector<std::pair<unsigned, SDValue>> RegsPairs;
  SDValue StackPtr;

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc()) {
      RegsPairs.push_back(std::make_pair(VA.getLocReg(), OutVals[i]));
    } else {
      assert(VA.isMemLoc());
      if (!StackPtr.getNode()) {
        StackPtr = DAG.getCopyFromReg(Chain, DL, M7004::AR15,
                                      getPointerTy(DAG.getDataLayout()));
      }
      unsigned LocMemOffset = VA.getLocMemOffset();
      SDValue PtrOff = DAG.getIntPtrConstant(LocMemOffset, DL);
      PtrOff = DAG.getNode(ISD::ADD, DL, getPointerTy(DAG.getDataLayout()),
                           StackPtr, PtrOff);
      Chain = DAG.getStore(Chain, DL, OutVals[i], PtrOff,
                           MachinePointerInfo::getStack(MF, LocMemOffset));
    }
  }

  // 处理被调函数符号
  GlobalAddressSDNode *N = dyn_cast<GlobalAddressSDNode>(Callee);
  Callee = DAG.getTargetGlobalAddress(N->getGlobal(), DL,
                                      getPointerTy(DAG.getDataLayout()));

  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);  // 0: Chain
  Ops.push_back(Callee); // 1: Callee

  unsigned NumArgs = Outs.size(); // 统计参数总数
  // Ops.push_back(DAG.getConstant(NumArgs, DL, MVT::i32));
  Ops.push_back(
      DAG.getTargetConstant(NumArgs, DL, MVT::i32)); // 这样类型和Callee类似

  SDValue Glue;
  for (auto &[reg, val] : RegsPairs) {
    Chain = DAG.getCopyToReg(Chain, DL, reg, val, Glue);
    Glue = Chain.getValue(1);
    Ops.push_back(DAG.getRegister(reg, val.getValueType()));
  }

  const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo();
  const uint32_t *Mask =
      TRI->getCallPreservedMask(DAG.getMachineFunction(), CallConv);
  Ops.push_back(DAG.getRegisterMask(Mask));
  if (Glue.getNode()) {
    Ops.push_back(Glue);
  }

  // ----------- 关键修改：处理返回值 -----------
  SmallVector<CCValAssign, 2> RVLocs;
  CCState RetCCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                    *DAG.getContext());
  RetCCInfo.AnalyzeCallResult(Ins, RetCC_M7004);

  SDVTList NodeTys;
  bool HasRet = !RVLocs.empty();
  SmallVector<EVT, 4> RetVTs;
  for (auto &VA : RVLocs)
    RetVTs.push_back(VA.getLocVT());

  if (HasRet) {
    SmallVector<EVT, 4> VTs = RetVTs;
    VTs.push_back(MVT::Other);
    VTs.push_back(MVT::Glue);
    NodeTys = DAG.getVTList(VTs);
  } else {
    NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  }

  SDValue CallNode = DAG.getNode(M7004ISD::Call, DL, NodeTys, Ops);

  // 处理返回值
  if (HasRet) {
    SDValue ChainOut = CallNode.getValue(RetVTs.size());
    SDValue GlueOut = CallNode.getValue(RetVTs.size() + 1);
    for (unsigned i = 0; i < RVLocs.size(); ++i) {
      unsigned RVReg = RVLocs[i].getLocReg();
      EVT RetVT = RVLocs[i].getLocVT();
      SDValue Val = DAG.getCopyFromReg(ChainOut, DL, RVReg, RetVT, GlueOut);
      InVals.push_back(Val);
      ChainOut = Val.getValue(1);
      GlueOut = Val.getValue(2);
    }
    return ChainOut;
  } else {
    return CallNode.getValue(0);
  }
}

SDValue M7004TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  /// 1. 分析ins的存储
  /// 2. 产生节点
  ///
  /// ir type, MVT, EVT
  /// ir type: i1 ~i128, f32, f64, struct, array
  /// MVT : machine value type, 寄存器的类型，架构所具体支持的类型，一般都是整型
  /// i8~i32 EVT : 扩展的value type，包含了架构所不支持的类型，比如 i3, i99

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_M7004);

  SDValue ArgValue;
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc()) {
      MVT RegVT = VA.getLocVT();
      //根据参数类型选择正确的寄存器类
      const TargetRegisterClass *RC = nullptr;
      if (RegVT == MVT::f32) {
        RC = &M7004::FP32SRRegClass;  // 浮点寄存器类
      } 
      else if(RegVT == MVT::i16)
      {
        RC = &M7004::GP16SRRegClass;  // 16位整型寄存器类
      }
      else if(RegVT == MVT::i32) {
        RC = &M7004::GP32SRRegClass;  // 32位整型寄
      }
      else {
        RC = &M7004::GP32SRRegClass;  // 整型寄存器类
      }
      Register Reg = MF.addLiveIn(VA.getLocReg(), RC);  // 使用正确的寄存器类
      ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, RegVT);
      InVals.push_back(ArgValue);
    }else {
      assert(VA.isMemLoc());
      // 获取参数类型
      MVT ValVT = VA.getValVT();
      // 获取参数的内存偏移量
      int Offset = VA.getLocMemOffset();
      // 创建栈对象,在栈上分配一个固定大小的内存区域，表示一个栈对象。
      int FI = MFI.CreateFixedObject(
          ValVT.getSizeInBits() / 8, Offset,
          true); // getSizeInBits():返回该类型的​​位大小​​（size
                 // in bits），即该类型在内存中占用的位数。i32 类型的
                 // ValVT.getSizeInBits() 返回 32。
      // 创建栈指针，创建一个栈指针，表示栈对象的地址。
      SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
      // 创建一个加载节点，表示从栈对象中加载数据。通过栈指针从栈对象中加载参数值。
      SDValue Val = DAG.getLoad(
          ValVT, DL, Chain, FIN,
          MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI));
      // 通过栈指针从栈对象中加载参数值。将加载的值添加到 InVals 中。
      InVals.push_back(Val);
    }
  }
  return Chain;
}

// 固定模版 OutVals拷贝到物理寄存器  通过调用约定决策CallConv
SDValue M7004TargetLowering::LowerReturn(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs /*输出参数*/,
    const SmallVectorImpl<SDValue> &OutVals /*输出值*/, const SDLoc &DL,
    SelectionDAG &DAG) const {
  /// 1. 返回物理寄存器（调用约定的限值）
  SmallVector<CCValAssign, 16> RVLocs; // 分析哪些寄存器可以用来做返回值

  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext() /*上下文*/);
  CCInfo.AnalyzeReturn(Outs, RetCC_M7004); // 分析返回值  RVLocs目前已有信息

  SDValue Glue;
  SmallVector<SDValue, 4> RetOps(1, Chain);
  for (unsigned i = 0, e = RVLocs.size(); i < e; ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    // 创建节点
    Chain = DAG.getCopyToReg(Chain, DL /*位置信息*/, VA.getLocReg(), OutVals[i],
                             Glue);
    Glue = Chain.getValue(1); // 设置成第一个输出值
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain;

  if (Glue.getNode()) {
    RetOps.push_back(Glue);
  }

  return DAG.getNode(M7004ISD::RET_GLUE, DL, MVT::Other, RetOps);
}

const char *M7004TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case M7004ISD::RET_GLUE:
    return "M7004ISD::RET_GLUE";
  case M7004ISD::Call:
    return "M7004ISD::Call";
  // case M7004ISD::HI:
  //   return "M7004ISD::HI";
  // case M7004ISD::LO:
  //   return "M7004ISD::LO";
  default:
    return nullptr;
  }
}

SDValue M7004TargetLowering::LowerSELECT_CC(SDValue Op,
                                            SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueVal = Op.getOperand(2);
  SDValue FalseVal = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();

  // SELECT_CC(LHS, RHS, TrueVal, FalseVal, CC)
  // 相当于: CC(LHS, RHS) ? TrueVal : FalseVal

  // 方法1：转换为 SELECT + SETCC
  SDValue Cmp = DAG.getSetCC(DL, MVT::i32, LHS, RHS, CC);
  return DAG.getSelect(DL, Op.getValueType(), Cmp, TrueVal, FalseVal);
}