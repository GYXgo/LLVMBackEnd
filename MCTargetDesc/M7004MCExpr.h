#ifndef M7004MCEXPR_H
#define M7004MCEXPR_H

#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCValue.h"

namespace llvm {
class M7004MCExpr : public MCTargetExpr {
public:
  enum M7004ExprKind { NONE};
  M7004MCExpr(M7004ExprKind Kind, const MCExpr *Expr) : Kind(Kind), Expr(Expr) {}

  //打印表达式到输出流
  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  //尝试将表达式求值为可重定位的值。这里直接返回 false，表示 M7004MCExpr 目前不支持直接求值。
  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAssembler *Asm,
                                 const MCFixup *Fixup) const override {
    return false;
  }
  //通知 Streamer 该表达式被使用
  void visitUsedExpr(MCStreamer &Streamer) const override {}
  //查找与该表达式相关联的 MCFragment。
  MCFragment *findAssociatedFragment() const override { return nullptr; }
  //修正 ELF 文件中 TLS 相关的符号。
  void fixELFSymbolsInTLSFixups(MCAssembler &) const override {}

private:
  const M7004ExprKind Kind;
  const MCExpr *Expr;
};
} // namespace llvm

#endif // M7004MCEXPR_H
