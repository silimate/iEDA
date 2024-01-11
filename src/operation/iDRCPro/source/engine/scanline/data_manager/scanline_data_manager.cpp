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

#include "scanline_data_manager.h"

namespace idrc {

template <typename T>
void ScanlineDataManager::deleteVectorElements(T& v)
{
  for (auto& element : v) {
    delete element;
    element = nullptr;
  }
}

ScanlineDataManager::~ScanlineDataManager()
{
  deleteVectorElements(_basic_points);
  deleteVectorElements(_scanline_points_vertical);
  deleteVectorElements(_scanline_points_horizontal);
}

/**
 * add boost points to scanline data manager
 * @param
 * GtlPoint : boost point
 * std::vector<GtlPoint> : point list for one polygon
 * std::vector<std::vector<GtlPoint>> : define all point list in the polygon list
 * net_id : indicate id
 */
void ScanlineDataManager::addData(std::vector<std::vector<ieda_solver::GtlPoint>>& polygons_points, int net_id)
{
  for (auto& polygon_points : polygons_points) {
    addPolygon(polygon_points, net_id);
  }
}

/// @brief add boost polygon to scanline data manager
/// @param polygon_points polygon endpoints clockwise
/// @param net_id polygon net_id
void ScanlineDataManager::addPolygon(std::vector<ieda_solver::GtlPoint>& polygon_points, int net_id)
{
  /// find left bottom point
  DrcBasicPoint* left_bottom_pt = nullptr;

  std::vector<DrcBasicPoint*> endpoints;
  endpoints.reserve(polygon_points.size());
  for (auto& vertex : polygon_points) {
    DrcBasicPoint* new_basic_pt = new DrcBasicPoint(vertex.x(), vertex.y(), net_id);
    endpoints.emplace_back(new_basic_pt);
    _basic_points_rtree.insert(std::make_pair(ieda_solver::BgPoint(new_basic_pt->get_x(), new_basic_pt->get_y()), new_basic_pt));
    if (left_bottom_pt == nullptr) {
      left_bottom_pt = new_basic_pt;
    } else {
      if ((*new_basic_pt) < (*left_bottom_pt)) {
        left_bottom_pt = new_basic_pt;
      }
    }
  }

  left_bottom_pt->set_as_start();

  auto endpoint1 = endpoints.begin();
  auto endpoint2 = endpoints.begin() + 1;
  ScanlinePoint* prev_segment_end = nullptr;
  ScanlinePoint* polygon_start = nullptr;
  for (; endpoint1 != endpoints.end(); ++endpoint1, ++endpoint2) {
    if (endpoint2 == endpoints.end()) {
      endpoint2 = endpoints.begin();
    }

    (*endpoint1)->set_next(*endpoint2);
    (*endpoint2)->set_prev(*endpoint1);

    // create scanline points
    ScanlinePoint* starting_point = nullptr;
    ScanlinePoint* ending_point = nullptr;
    if ((*endpoint1)->get_x() == (*endpoint2)->get_x()) {
      // vertical
      bool is_forward_edge = (*endpoint1)->get_y() > (*endpoint2)->get_y();
      starting_point = new ScanlinePoint(*endpoint1, is_forward_edge, !is_forward_edge);
      ending_point = new ScanlinePoint(*endpoint2, is_forward_edge, is_forward_edge);
      _scanline_points_vertical.emplace_back(starting_point);
      _scanline_points_vertical.emplace_back(ending_point);
    } else if ((*endpoint1)->get_y() == (*endpoint2)->get_y()) {
      // horizontal
      bool is_forward_edge = (*endpoint1)->get_x() < (*endpoint2)->get_x();
      starting_point = new ScanlinePoint(*endpoint1, is_forward_edge, is_forward_edge);
      ending_point = new ScanlinePoint(*endpoint2, is_forward_edge, !is_forward_edge);
      _scanline_points_horizontal.emplace_back(starting_point);
      _scanline_points_horizontal.emplace_back(ending_point);
    } else {
      std::cout << "scanline error: polygon is not horizontal or vertical" << std::endl;
    }
    starting_point->set_pair(ending_point);
    ending_point->set_pair(starting_point);

    // match points in both horizontal and vertical
    if (prev_segment_end != nullptr) {
      starting_point->set_orthogonal_point(prev_segment_end);
      prev_segment_end->set_orthogonal_point(starting_point);
    } else {
      polygon_start = starting_point;
    }

    if (endpoint2 == endpoints.begin()) {
      polygon_start->set_orthogonal_point(ending_point);
      ending_point->set_orthogonal_point(polygon_start);
    }

    prev_segment_end = ending_point;
  }

  /// save data
  std::copy(endpoints.begin(), endpoints.end(), std::back_inserter(_basic_points));
}

/// @brief sort scanline points in both horizontal and vertical direction
void ScanlineDataManager::sortEndpoints()
{
  std::sort(_scanline_points_horizontal.begin(), _scanline_points_horizontal.end(), CompareScanlinePointByX());
  std::sort(_scanline_points_vertical.begin(), _scanline_points_vertical.end(), CompareScanlinePointByY());
}

}  // namespace idrc