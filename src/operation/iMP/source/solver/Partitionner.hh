/**
 * @file Partitionner.hh
 * @author Fuxing Huang (fxxhuang@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-07-11
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef IMP_PARTITIONNER_H
#define IMP_PARTITIONNER_H
#include <fstream>
#include <string>
#include <vector>
using std::vector;
namespace imp {

class Partitionner
{
 public:
  static vector<size_t> hmetisSolve(size_t num_vertexs, size_t num_hedges, const vector<size_t>& eptr, const vector<size_t>& eind,
                                    size_t nparts, float ufactor, const vector<int64_t>& vwgt = {}, const vector<int64_t>& hewgt = {},
                                    int n_runs = 5, int seed = 0);

 private:
  Partitionner() = delete;
  ~Partitionner() = delete;
};

}  // namespace imp

#endif