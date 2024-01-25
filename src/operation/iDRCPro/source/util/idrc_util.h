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
#pragma once

#include <vector>

#include "drc_basic_point.h"
#include "idrc_engine.h"

namespace idrc {

class DrcUtil
{
 public:
  // 叉乘
  template <typename T>
  static int crossProduct(T& first_coord, T& second_coord, T& third_coord)
  {
    return (second_coord.get_x() - first_coord.get_x()) * (third_coord.get_y() - first_coord.get_y())
           - (second_coord.get_y() - first_coord.get_y()) * (third_coord.get_x() - first_coord.get_x());
  }

  // 用叉乘判断是否是凸角
  template <typename T>
  static bool isConvexCorner(T& first_coord, T& second_coord, T& third_coord)
  {
    return crossProduct(first_coord, second_coord, third_coord) < 0;
  }

  // 用叉乘判断是否是凹角
  template <typename T>
  static bool isConcaveCorner(T& first_coord, T& second_coord, T& third_coord)
  {
    return crossProduct(first_coord, second_coord, third_coord) > 0;
  }

  // 获得 polygon 的所有点，按顺时针组织
  static std::vector<ieda_solver::GtlPoint> getPolygonPoints(DrcBasicPoint* point)
  {
    std::vector<ieda_solver::GtlPoint> point_list{{point->get_x(), point->get_y()}};
    auto* iter_pt = point->get_next();
    while (nullptr != iter_pt && iter_pt != point) {
      point_list.emplace_back(iter_pt->get_x(), iter_pt->get_y());

      iter_pt = iter_pt->get_next();
    }

    return point_list;
  }

  // 方向取反
  static DrcDirection oppositeDirection(DrcDirection direction)
  {
    switch (direction) {
      case DrcDirection::kUp:
        return DrcDirection::kDown;
      case DrcDirection::kDown:
        return DrcDirection::kUp;
      case DrcDirection::kLeft:
        return DrcDirection::kRight;
      case DrcDirection::kRight:
        return DrcDirection::kLeft;
      default:
        return DrcDirection::kNone;
    }
  }

  // 线段左边为多边形外部
  static DrcDirection outsidePolygonDirection(DrcBasicPoint* point_prev, DrcBasicPoint* point_next)
  {
    DrcDirection direction = point_prev->direction(point_next);
    switch (direction) {
      case DrcDirection::kUp:
        return DrcDirection::kLeft;
      case DrcDirection::kDown:
        return DrcDirection::kRight;
      case DrcDirection::kLeft:
        return DrcDirection::kDown;
      case DrcDirection::kRight:
        return DrcDirection::kUp;
      default:
        return DrcDirection::kNone;
    }
  }

  // 获得正交方向
  static std::pair<DrcDirection, DrcDirection> orthogonalDirections(DrcDirection direction)
  {
    switch (direction) {
      case DrcDirection::kUp:
      case DrcDirection::kDown:
        return std::make_pair(DrcDirection::kLeft, DrcDirection::kRight);
      case DrcDirection::kLeft:
      case DrcDirection::kRight:
        return std::make_pair(DrcDirection::kUp, DrcDirection::kDown);
      default:
        return std::make_pair(DrcDirection::kNone, DrcDirection::kNone);
    }
  }

  // 获得向某方向行走特定距离的坐标
  static std::pair<int, int> transformPoint(int x, int y, DrcDirection direction, int distance)
  {
    switch (direction) {
      case DrcDirection::kUp:
        return std::make_pair(x, y + distance);
      case DrcDirection::kDown:
        return std::make_pair(x, y - distance);
      case DrcDirection::kLeft:
        return std::make_pair(x - distance, y);
      case DrcDirection::kRight:
        return std::make_pair(x + distance, y);
      default:
        return std::make_pair(x, y);
    }
  }

 private:
};

}  // namespace idrc