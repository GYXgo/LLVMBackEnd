

#include "M7004MCExpr.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

void M7004MCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {

  Expr->print(OS, MAI, true);

}