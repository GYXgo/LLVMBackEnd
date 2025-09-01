
#include "M7004TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

namespace llvm {
  //返回一个全局唯一的 Target 对象，代表 M7004 架构。
  Target &getM7004Target() {
    static Target M7004Target;
    return M7004Target;
  }

} // end namespace llvm

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeM7004TargetInfo() {
  RegisterTarget<Triple::M7004> X(getM7004Target(), "m7004",
                                      "M7004 32-bit", "M7004");
}
