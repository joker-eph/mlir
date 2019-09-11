// RUN: toyc-ch6 %s -emit=mlir-affine -opt 2>&1 | FileCheck %s

func @main() {
  %0 = "toy.constant"() {value = dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>} : () -> tensor<2x3xf64>
  %2 = "toy.transpose"(%0) : (tensor<2x3xf64>) -> tensor<2x3xf64>
  %3 = "toy.mul"(%2, %2) : (tensor<2x3xf64>, tensor<2x3xf64>) -> tensor<2x3xf64>
  "toy.print"(%3) : (tensor<2x3xf64>) -> ()
  "toy.return"() : () -> ()
}

// CHECK-LABEL: func @main()
// CHECK:         [[VAL_0:%.*]] = constant 0 : index
// CHECK:         [[VAL_1:%.*]] = constant 1 : index
// CHECK:         [[VAL_2:%.*]] = constant 2 : index
// CHECK:         [[VAL_3:%.*]] = constant 1.000000e+00 : f64
// CHECK:         [[VAL_4:%.*]] = constant 2.000000e+00 : f64
// CHECK:         [[VAL_5:%.*]] = constant 3.000000e+00 : f64
// CHECK:         [[VAL_6:%.*]] = constant 4.000000e+00 : f64
// CHECK:         [[VAL_7:%.*]] = constant 5.000000e+00 : f64
// CHECK:         [[VAL_8:%.*]] = constant 6.000000e+00 : f64
// CHECK:         [[VAL_9:%.*]] = alloc() : memref<2x3xf64>
// CHECK:         [[VAL_10:%.*]] = alloc() : memref<2x3xf64>
// CHECK:         affine.store [[VAL_3]], [[VAL_10]]{{\[}}[[VAL_0]], [[VAL_0]]] : memref<2x3xf64>
// CHECK:         affine.store [[VAL_4]], [[VAL_10]]{{\[}}[[VAL_0]], [[VAL_1]]] : memref<2x3xf64>
// CHECK:         affine.store [[VAL_5]], [[VAL_10]]{{\[}}[[VAL_0]], [[VAL_2]]] : memref<2x3xf64>
// CHECK:         affine.store [[VAL_6]], [[VAL_10]]{{\[}}[[VAL_1]], [[VAL_0]]] : memref<2x3xf64>
// CHECK:         affine.store [[VAL_7]], [[VAL_10]]{{\[}}[[VAL_1]], [[VAL_1]]] : memref<2x3xf64>
// CHECK:         affine.store [[VAL_8]], [[VAL_10]]{{\[}}[[VAL_1]], [[VAL_2]]] : memref<2x3xf64>
// CHECK:         affine.for [[VAL_11:%.*]] = 0 to 2 {
// CHECK:           affine.for [[VAL_12:%.*]] = 0 to 3 {
// CHECK:             [[VAL_13:%.*]] = affine.load [[VAL_10]]{{\[}}[[VAL_12]], [[VAL_11]]] : memref<2x3xf64>
// CHECK:             [[VAL_14:%.*]] = mulf [[VAL_13]], [[VAL_13]] : f64
// CHECK:             affine.store [[VAL_14]], [[VAL_9]]{{\[}}[[VAL_11]], [[VAL_12]]] : memref<2x3xf64>
// CHECK:         "toy.print"([[VAL_9]]) : (memref<2x3xf64>) -> ()
// CHECK:         dealloc [[VAL_10]] : memref<2x3xf64>
// CHECK:         dealloc [[VAL_9]] : memref<2x3xf64>
