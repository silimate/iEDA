// ***************************************************************************************
// Copyright (c) 2023-2025 Peng Cheng Laboratory
// Copyright (c) 2023-2025 Institute of Computing Technology, Chinese Academy of Sciences
// Copyright (c) 2023-2025 Beijing Institute of Open Source Chip
//
// iEDA is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
// http://license.coscl.org.cn/MulanPSL2
//
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
//
// See the Mulan PSL v2 for more details.
// ***************************************************************************************
/**
 * @file ModelFactory.cc
 * @author Dawn Li (dawnli619215645@gmail.com)
 */
#include "ModelFactory.hh"

#include <Eigen/Dense>
#include <cmath>
#include <unsupported/Eigen/Polynomials>

#ifdef PY_MODEL
#include "PyModel.h"
#endif
namespace icts {

std::vector<double> ModelFactory::solvePolynomialRealRoots(const std::vector<double>& coeffs) const
{
  int poly_order = coeffs.size() - 1;
  Eigen::VectorXd poly_coeffs(poly_order + 1);
  for (int i = 0; i < poly_order + 1; i++) {
    poly_coeffs(i) = coeffs[poly_order - i];
  }
  Eigen::PolynomialSolver<double, Eigen::Dynamic> solver;
  solver.compute(poly_coeffs);
  std::vector<double> result;
  solver.realRoots(result);
  return result;
}

std::vector<double> ModelFactory::cppLinearModel(const std::vector<std::vector<double>>& x, const std::vector<double>& y) const
{
  int m = x.size();
  int n = x[0].size();

  Eigen::MatrixXd t_x(n, m + 1);
  Eigen::VectorXd t_y(n);

  for (int i = 0; i < n; i++) {
    t_x(i, 0) = 1;
    for (int j = 0; j < m; j++) {
      t_x(i, j + 1) = x[j][i];
    }
    t_y(i) = y[i];
  }

  Eigen::VectorXd coeffs = (t_x.transpose() * t_x).ldlt().solve(t_x.transpose() * t_y);

  std::vector<double> result(m + 1);
  for (int i = 0; i < m + 1; i++) {
    result[i] = coeffs(i);
  }

  return result;
}

double ModelFactory::criticalBufWireLen(const double& alpha, const double& beta, const double& gamma, const double& r, const double& c,
                                        const double& cap_pin)
{
  return 2 * std::sqrt((beta * cap_pin + gamma) / (r * c * (std::log(9) * alpha + 1)));
}

std::pair<double, double> ModelFactory::criticalSteinerWireLen(const double& alpha, const double& beta, const double& gamma,
                                                               const double& r, const double& c, const double& cap_pin,
                                                               const double& cap_load, const size_t& step)
{
  if (alpha <= 0 || beta <= 0 || gamma <= 0 || r <= 0 || c <= 0 || cap_load <= 0 || step <= 0) {
    assert(false);
    return {0, 0};
  }
  /**
   * f(x,l) = 1/2*r*c*[(2+alpha*ln9)*x^2-2x]*l^2+{r*c*[(1+alpha*ln9)*cap_pin-cap_load]+gamma <= 0
   *
   */
  double critical_wl = std::numeric_limits<double>::max();
  double critical_x = 0;
  double max_x = 2.0 / (2.0 + alpha * std::log(9));
  for (size_t i = 0; i < step; ++i) {
    auto x = 1.0 / step * (i + 1);
    if (x > max_x) {
      break;
    }
    double a = 0.5 * r * c * ((2 + alpha * std::log(9)) * x * x - 2 * x);
    double b = r * c * ((1 + alpha * std::log(9)) * cap_pin - cap_load);
    double c = gamma;
    double delta = b * b - 4 * a * c;
    if (delta < 0) {
      continue;
    }
    double l1 = (-b - std::sqrt(delta)) / (2 * a);
    if (l1 < critical_wl) {
      critical_wl = l1;
      critical_x = x;
    }
  }
  return {critical_wl, critical_x};
}
#ifdef PY_MODEL
/**
 * @brief Python interface for timing model
 *
 * @param x (m x n)
 * @param y (n)
 */

double ModelBase::predict(const std::vector<double>& x) const
{
  return pyPredict(x, _model);
}

ModelBase* ModelFactory::pyFit(const std::vector<std::vector<double>>& x, const std::vector<double>& y, const FitType& fit_type) const
{
  PyObject* model;
  switch (fit_type) {
    case FitType::kLinear:
      model = pyLinearModel(x, y);
      break;
    case FitType::kCatBoost:
      model = pyCatBoostModel(x, y);
      break;
    case FitType::kXgBoost:
      model = pyXGBoostModel(x, y);
      break;
    default:
      model = pyLinearModel(x, y);
      break;
  }
  return new ModelBase(model);
}

ModelBase* ModelFactory::pyLoad(const std::string& model_path) const
{
  auto* model = pyLoadModel(model_path);
  return new ModelBase(model);
}
#endif
}  // namespace icts