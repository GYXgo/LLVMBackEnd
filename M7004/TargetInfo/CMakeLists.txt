add_llvm_component_library(LLVMM7004Info
M7004TargetInfo.cpp

LINK_COMPONENTS
MC #必须要写这个
Support
)