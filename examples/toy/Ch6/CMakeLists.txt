
set(LLVM_TARGET_DEFINITIONS include/toy/Ops.td)
mlir_tablegen(include/toy/Ops.h.inc -gen-op-decls)
mlir_tablegen(include/toy/Ops.cpp.inc -gen-op-defs)
add_public_tablegen_target(ToyCh6OpsIncGen)

set(LLVM_LINK_COMPONENTS
  Support
  )

add_toy_chapter(toyc-ch6
  toyc.cpp
  parser/AST.cpp
  mlir/LowerToAffineLoops.cpp
  mlir/MLIRGen.cpp
  mlir/Dialect.cpp
  )
include_directories(include/)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include/)
add_dependencies(toyc-ch6 ToyCh6OpsIncGen)
target_link_libraries(toyc-ch6
  PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRTransforms)
