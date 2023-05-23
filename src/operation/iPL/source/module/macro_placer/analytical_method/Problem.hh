/**
 * @file Problem.hh
 * @author Fuxing Huang (fxxhuang@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-05-16
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef IPL_MP_PROBLEM_H
#define IPL_MP_PROBLEM_H
#include "eigen3/Eigen/Dense"
using Eigen::MatrixXf;
using Eigen::VectorXf;
namespace ipl {

class Problem
{
 public:
  Problem(/* args */){};
  virtual ~Problem(){};
  virtual void evaluate(const MatrixXf& variable, MatrixXf& gradient, float& cost, int iter) const = 0;
  // virtual void updateParameter(VectorXf& parameter, int iter) = 0;
  // virtual MatrixXf hessianMatrix() = 0;
  virtual float getLowerBound(int row, int col) const { return -100000.0; }
  virtual float getUpperBound(int row, int col) const { return 100000.0; };
  virtual int variableMatrixRows() const = 0;
  virtual int variableMatrixcols() const = 0;

 protected:
  // MatrixXf _constant;
};

}  // namespace ipl

#endif  // IPL_MP_PROBLEM_H