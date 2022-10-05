/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/compiler/tf2xla/lib/scatter.h"

#include <functional>
#include <memory>
#include <vector>

#include "absl/types/span.h"
#include "tensorflow/compiler/tf2xla/lib/util.h"
#include "tensorflow/compiler/xla/client/lib/arithmetic.h"
#include "tensorflow/compiler/xla/client/xla_builder.h"
#include "tensorflow/compiler/xla/literal.h"
#include "tensorflow/compiler/xla/shape_util.h"
#include "tensorflow/compiler/xla/status_macros.h"
#include "tensorflow/compiler/xla/util.h"
#include "tensorflow/core/lib/core/errors.h"

namespace tensorflow {

StatusOr<xla::XlaOp> XlaScatter(
    const xla::XlaOp& buffer, const xla::XlaOp& updates,
    const xla::XlaOp& indices, bool indices_are_vectors,
    const std::function<xla::XlaOp(xla::XlaOp, xla::XlaOp, xla::XlaBuilder*)>&
        combiner,
    xla::XlaBuilder* builder) {
  TF_ASSIGN_OR_RETURN(xla::Shape buffer_shape, builder->GetShape(buffer));
  TF_ASSIGN_OR_RETURN(xla::Shape updates_shape, builder->GetShape(updates));
  TF_ASSIGN_OR_RETURN(xla::Shape indices_shape, builder->GetShape(indices));
  absl::Span<const int64_t> indices_dims = indices_shape.dimensions();

  // If the indices are N-dimensional, the minor dimension of indices contains
  // the indices to update. Otherwise the indices are all scalars.
  int64_t num_index_dims = 1;
  if (indices_are_vectors) {
    TF_RET_CHECK(!indices_dims.empty());
    num_index_dims = indices_dims.back();
    if (num_index_dims > buffer_shape.rank()) {
      return errors::InvalidArgument(
          "The size of the minor dimension of the indices (shape: ",
          xla::ShapeUtil::HumanString(indices_shape),
          ") must be <= the rank of the buffer (shape: ",
          xla::ShapeUtil::HumanString(buffer_shape), ")");
    }
    indices_dims.remove_suffix(1);
  }

  int64_t num_indices = 1;
  for (int64_t dim : indices_dims) {
    num_indices *= dim;
  }

  // Degenerate case: nothing to update. Return the buffer unchanged.
  if (num_indices == 0) {
    return buffer;
  }

  // If any of the indexed dimensions are zero in the buffer, the update cannot
  // succeed since it updates a slice of size 1.
  for (int64_t i = 0; i < num_index_dims; ++i) {
    if (xla::ShapeUtil::GetDimension(buffer_shape, i) == 0) {
      return errors::InvalidArgument("Scatter dimension ", i,
                                     " is of size zero in tensor with shape ",
                                     xla::ShapeUtil::HumanString(buffer_shape));
    }
  }

  // Example of a 1-D scatter that updates two [3,1] tensors in a tensor of
  // shape [3,3]:
  // NOTE: ***This case will not be generated by any of the tf.scatter ops.***
  //
  //   operand = s32[3,3] parameter(0)
  //   indices = s32[2] parameter(1)
  //   updates = s32[3,2] parameter(2)
  //   scatter = s32[3,3] scatter(operand, indices, updates),
  //       to_apply=update_computation,
  //       update_window_dims={0},
  //       inserted_window_dims={1},
  //       scatter_dims_to_operand_dims={1},
  //       index_vector_dim=1
  //
  //
  // Example of a 1-D scatter that updates two [1,3] tensors in a tensor of
  // shape [3,3]:
  //
  //   operand = s32[3,3] parameter(0)
  //   indices = s32[2] parameter(1)
  //   updates = s32[2,3] parameter(2)
  //   scatter = s32[3,3] scatter(operand, indices, updates),
  //       to_apply=update_computation,
  //       update_window_dims={1},
  //       inserted_window_dims={0},
  //       scatter_dims_to_operand_dims={0},
  //       index_vector_dim=1
  //
  //
  // Example of an N-D scatter updating slices of shape [1,1,2] in a tensor of
  // shape [3,3,2]
  //
  //   operand = s32[3,3,2] parameter(0)
  //   indices = s32[2,2] parameter(1)
  //   updates = s32[2,2] parameter(2)
  //   scatter = s32[3,3,2] scatter(operand, indices, updates),
  //       to_apply=update_computation,
  //       update_window_dims={1},
  //       inserted_window_dims={0,1},
  //       scatter_dims_to_operand_dims={0,1},
  //       index_vector_dim=1
  //
  //
  // Example of a scatter updating slices of shape [] in a tensor of shape [1,1]
  //
  //   operand = s32[1,1] parameter(0)
  //   indices = s32[1] parameter(1)
  //   updates = s32[1] parameter(2)
  //   scatter = s32[1,1] scatter(operand, indices, updates),
  //       to_apply=update_computation,
  //       update_window_dims={},
  //       inserted_window_dims={0,1},
  //       scatter_dims_to_operand_dims={0},
  //       index_vector_dim=1
  // Note that updates operand would be broadcasted into [1] in this case.
  //

  xla::ScatterDimensionNumbers dim_numbers;
  dim_numbers.set_index_vector_dim(indices_are_vectors
                                       ? indices_shape.dimensions_size() - 1
                                       : indices_shape.dimensions_size());

  int64_t updates_rank = updates_shape.rank();
  int64_t buffer_rank = buffer_shape.rank();
  int64_t num_window_dims_in_updates = buffer_rank - num_index_dims;

  // If the rank of `updates` is 0 and does not match the expected rank of
  // updates, broadcast `updates` to the expected shape of updates.
  auto new_updates = updates;
  std::vector<int64_t> expected_updates_dims(indices_dims.begin(),
                                             indices_dims.end());
  for (int64_t dim = num_index_dims; dim < buffer_rank; ++dim) {
    expected_updates_dims.push_back(buffer_shape.dimensions(dim));
  }
  int64_t expected_updates_rank = expected_updates_dims.size();
  if (updates_rank == 0 && expected_updates_rank != 0) {
    new_updates = xla::Broadcast(updates, expected_updates_dims);
    TF_ASSIGN_OR_RETURN(updates_shape, builder->GetShape(new_updates));
    updates_rank = updates_shape.rank();
  }

  if (updates_rank > 0) {
    for (int64_t i = (updates_rank - num_window_dims_in_updates);
         i < updates_rank; ++i) {
      dim_numbers.add_update_window_dims(i);
    }
  }

  for (int64_t i = 0; i < num_index_dims; ++i) {
    dim_numbers.add_inserted_window_dims(i);
    dim_numbers.add_scatter_dims_to_operand_dims(i);
  }

  // Build the combiner computation.
  xla::XlaComputation combiner_computation;
  {
    xla::XlaBuilder cb("scatter-combiner");
    auto xla_scalar_shape =
        xla::ShapeUtil::MakeShape(buffer_shape.element_type(), {});
    auto p0 = xla::Parameter(&cb, 0, xla_scalar_shape, "p0");
    auto p1 = xla::Parameter(&cb, 1, xla_scalar_shape, "p1");
    if (combiner) {
      combiner(p0, p1, &cb);
    }
    combiner_computation = cb.Build().ConsumeValueOrDie();
  }

  VLOG(3) << "Scatter op:";
  VLOG(3) << "  Input: " << xla::ShapeUtil::HumanString(buffer_shape);
  VLOG(3) << "  Indices: " << xla::ShapeUtil::HumanString(indices_shape);
  VLOG(3) << "  Updates: " << xla::ShapeUtil::HumanString(updates_shape);
  VLOG(3) << "  Scatter Dimension Numbers: ";
  VLOG(3) << "    index_vector_dim: " << dim_numbers.index_vector_dim();
  VLOG(3) << "    update_window_dims: ["
          << absl::StrJoin(dim_numbers.update_window_dims(), ",") << "]";
  VLOG(3) << "    inserted_window_dims: ["
          << absl::StrJoin(dim_numbers.inserted_window_dims(), ",") << "]";
  VLOG(3) << "    scatter_dims_to_operand_dims: ["
          << absl::StrJoin(dim_numbers.scatter_dims_to_operand_dims(), ",")
          << "]";

  return xla::Scatter(buffer, indices, new_updates, combiner_computation,
                      dim_numbers);
}

}  // namespace tensorflow