/**
 * @file IRMatrix.cc
 * @author simin tao (taosm@pcl.ac.cn)
 * @brief build matrix of IR data.
 * @version 0.1
 * @date 2024-04-12
 */

#include "IRMatrix.hh"

namespace iir {

/**
 * @brief build conductance matrix of one net.
 *
 * @param one_net_matrix_data
 * @param node_num
 * @return Eigen::Map<Eigen::SparseMatrix<double>>
 */
Eigen::Map<Eigen::SparseMatrix<double>> IRMatrix::buildConductanceMatrix(
    RustNetConductanceData& one_net_matrix_data) {
  auto node_num = one_net_matrix_data.node_num;
  Eigen::SparseMatrix<double> mat(node_num, node_num);

  std::vector<Eigen::Triplet<double>> triplets;
  RustMatrix* one_data;
  FOREACH_VEC_ELEM(&one_net_matrix_data.g_matrix_vec, RustMatrix, one_data) {
    triplets.emplace_back(
        Eigen::Triplet<double>(one_data->row, one_data->col, one_data->data));
  }
  mat.setFromTriplets(triplets.begin(), triplets.end());

  Eigen::Map<Eigen::SparseMatrix<double>> G_matrix(
      mat.rows(), mat.cols(), mat.nonZeros(), mat.outerIndexPtr(),
      mat.innerIndexPtr(), mat.valuePtr());

  return G_matrix;
}

/**
 * @brief build current vector from rust data.
 *
 * @param instance_current_vec rust hash map of instance_current.
 * @return Eigen::VectorXd
 */
Eigen::VectorXd IRMatrix::buildCurrentVector(void* instance_current_map,
                                             std::size_t node_num) {
  Eigen::VectorXd J_vector;
  J_vector.setZero(node_num);

  auto* iter = create_hashmap_iterator(instance_current_map);
  uintptr_t node_id;
  double current_value;
  while(hashmap_iterator_next(iter, &node_id, &current_value)) {
    J_vector(node_id) = current_value;
  }

  destroy_hashmap_iterator(iter);

  return J_vector;
}

}  // namespace iir