
add_llvm_component_library(LLVMM7004Desc
        M7004MCTargetDesc.cpp
        M7004MCAsmInfo.cpp
        M7004InstPrinter.cpp
        M7004MCExpr.cpp

        LINK_COMPONENTS
        MC
        M7004Info
        Support
        TargetParser

        ADD_TO_COMPONENT
        M7004
)

