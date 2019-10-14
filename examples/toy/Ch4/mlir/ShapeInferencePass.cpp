//===- ShapeInferencePass.cpp - Shape Inference -===//
//
// Copyright 2019 The MLIR Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =============================================================================
//
// This file implements a Function level pass performing interprocedural
// propagation of array shapes through function specialization.
//
//===----------------------------------------------------------------------===//

#include "include/toy/Passes.h"
#include "mlir/Analysis/Verifier.h"
#include "mlir/Dialect/StandardOps/Ops.h"
#include "mlir/IR/BlockAndValueMapping.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/StandardTypes.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Support/LogicalResult.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/StandardTypes.h"
#include <algorithm>

#define DEBUG_TYPE "shape-inference"

using llvm::raw_ostream;
using llvm::MutableArrayRef;
using llvm::SmallVector;
using llvm::SmallVectorImpl;
using llvm::StringRef;
using llvm::Twine;
using namespace mlir;

namespace {

#include "toy/Ops.h.inc"
#include "toy/Ops.cpp.inc"
#include "toy/ShapeInferenceOpInterfaces.h.inc"
#include "toy/ShapeInferenceOpInterfaces.cpp.inc"

/// The ShapeInferencePass is a FunctionPass that performs intra-procedural shape inference.
///
///    Algorithm:
///
///   1) Build a worklist containing all the operations that are returning
///      a generic Toy array: these are the operations that need shape
///      inference.
///   2) Iterate on the worklist:
///     a) find an operation to process: the next ready operation in the
///        worklist has all of its arguments non-generic,
///     b) if no operation is found, break out of the loop,
///     c) remove the operation from the worklist,
///     d) infer the shape of its output from the arguments type.
///   3) If the worklist is empty, the algorithm succeeded and we infer the
///      return type for the function from the return operation.
///
class ShapeInferencePass : public mlir::FunctionPass<ShapeInferencePass> {
public:
  bool returnsGenericArray(Operation *op) {
    if (op->getNumResults() == 1) {
      auto arrayTy = op->getResult(0)->getType().cast<RankedTensorType>();
      return arrayTy.getShape().empty();
    }
    return false;
  }

  void runOnFunction() override {

    auto f = getFunction();

    // Populate the worklist with the operations that need shape inference:
    // these are operations that return a generic array.
    llvm::SmallPtrSet<mlir::Operation *, 16> opWorklist;
    f.walk([&](mlir::Operation *op) {
      if (returnsGenericArray(op))
      {
        opWorklist.insert(op);
      }
    });

    // Iterate on the operations in the worklist until all operations have been
    // inferred or no change happened (fix point).
    while (!opWorklist.empty()) {
      // Find the next operation ready for inference, that is an operation
      // with all operands already resolved (non-generic).
      auto nextop = llvm::find_if(opWorklist, [this](Operation *op){return this->returnsGenericArray(op);});

      if (nextop == opWorklist.end())
        break; // failure: no operations can be inferred.

      Operation *op = *nextop;
      opWorklist.erase(op);
      LLVM_DEBUG(llvm::dbgs() << "Inferring shape for: " << *op << "\n");
      auto toyOp = dyn_cast<Toy_Op>(op); 
      toyOp.inferShapes();
    }

    // If the operation worklist isn't empty, this indicates a failure.
    if (!opWorklist.empty()) {
      signalPassFailure();
      auto diag = f.emitError("Shape inference failed, ")
                  << opWorklist.size() << " operations couldn't be inferred\n";
    }
  }
};
}
namespace mlir {
/// Create a Shape Inference pass.
std::unique_ptr<mlir::Pass> createShapeInferencePass() {
  return std::make_unique<ShapeInferencePass>();
}
}
