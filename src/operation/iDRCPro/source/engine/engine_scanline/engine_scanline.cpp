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

#include "engine_scanline.h"

#include "condition_manager.h"
#include "idrc_data.h"
#include "idrc_util.h"

namespace std {
template <typename T>
inline void hash_combine(std::size_t& seed, T const& v)
{
  seed ^= hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
template <typename U, typename V>
struct hash<std::pair<U, V>>
{
  size_t operator()(std::pair<U, V> const& tt) const
  {
    size_t seed = 0;
    hash_combine(seed, tt.first);
    hash_combine(seed, tt.second);
    return seed;
  }
};
}  // namespace std

namespace idrc {

DrcEngineScanline::~DrcEngineScanline()
{
  if (_preprocess != nullptr) {
    delete _preprocess;
    _preprocess = nullptr;
  }
}

/// @brief process scanline in both of the directions
void DrcEngineScanline::doScanline()
{
  scan(ScanlineTravelDirection::kHorizontal);
  scan(ScanlineTravelDirection::kVertical);
}

/// @brief process scanline in one direction
/// @param direction scanline travel direction
void DrcEngineScanline::scan(ScanlineTravelDirection direction)
{
  ScanlineStatus scanline_status(direction, _preprocess);

  while (scanline_status.endpoints_it != scanline_status.endpoints_end) {
    // add all endpoints in current bucket to scanline status
    addCurrentBucketToScanline(scanline_status);

    // process scanline status once
    processScanlineStatus(scanline_status);

    // remove ending points
    removeEndingPoints(scanline_status);
  }
}

/// @brief add all points with same travel direction coordinate to scanline status
/// @param status scanline status
// TODO: deal with overlap, one direction is enough
void DrcEngineScanline::addCurrentBucketToScanline(ScanlineStatus& status)
{
  status.prepareNewBucket();

  ScanlinePoint* current_activate_point = nullptr;
  auto record_overlap = [&current_activate_point](ScanlinePoint* point) {
    if (!current_activate_point && !point->get_is_forward()) {
      current_activate_point = point;
    } else if (current_activate_point && (point)->get_is_forward() && current_activate_point->get_id() == point->get_id()) {
      current_activate_point = nullptr;
    } else if (current_activate_point && current_activate_point->get_id() != point->get_id()) {
      // TODO: overlap
      // auto gtl_pts_1 = DrcUtil::getPolygonPoints(current_activate_point->get_point());
      // auto gtl_pts_2 = DrcUtil::getPolygonPoints(point->get_point());
      // auto polygon_1 = ieda_solver::GtlPolygon(gtl_pts_1.begin(), gtl_pts_1.end());
      // auto polygon_2 = ieda_solver::GtlPolygon(gtl_pts_2.begin(), gtl_pts_2.end());
      // std::vector<ieda_solver::GtlPolygon> overlap_list;
      // ieda_solver::GtlPolygonSet intersection = polygon_1 & polygon_2;
      // intersection.get(overlap_list);
      // int a = 0;
    }
  };

  // add new points to scanline status
  auto bucket_end = status.nextBucketEnd();
  auto scanline_status_it = status.status_points.begin();
  bool is_first_insert = true;
  for (; status.endpoints_it != bucket_end; ++status.endpoints_it) {
    ScanlinePoint* current_point = *status.endpoints_it;

    if (current_point->get_is_start()) {
      // find correct position to insert
      while (scanline_status_it != status.status_points.end() && (*status.compare_scanline_point)(*scanline_status_it, current_point)) {
        record_overlap(*scanline_status_it);
        ++scanline_status_it;
      }
      scanline_status_it = status.status_points.insert(scanline_status_it, current_point);
    } else {
      // point is ending point, replace pair starting point in scanline status
      auto* current_pair_point = current_point->get_pair();
      while (scanline_status_it != status.status_points.end() && *scanline_status_it != current_pair_point) {
        record_overlap(*scanline_status_it);
        ++scanline_status_it;
      }
      *scanline_status_it = current_point;
    }

    // first insert, mark range begin
    if (is_first_insert) {
      status.insert_begin = scanline_status_it;
      is_first_insert = false;
      for (int i = 0; i < 2 && status.insert_begin != status.status_points.begin(); ++i) {
        --status.insert_begin;
      }
    }
  }

  // mark range end
  status.insert_end = scanline_status_it;
  for (int i = 0; i < 3 && status.insert_end != status.status_points.end(); ++i) {
    ++status.insert_end;
  }
}

/// @brief determine segment type while scanline process
/// @param status scanline status
/// @param point_forward forward point of current segment
/// @param point_backward backward point of current segment
/// @return segment type
ScanlineSegmentType DrcEngineScanline::judgeSegmentType(ScanlineStatus& status, ScanlinePoint* point_forward, ScanlinePoint* point_backward)
{
  ScanlineSegmentType result_type = ScanlineSegmentType::kNone;
  if (point_forward->get_is_new() && point_backward->get_is_new()
      && (point_forward->get_point()->get_next() == point_backward->get_point()
          || point_backward->get_point()->get_next() == point_forward->get_point())) {
    if (point_forward->get_is_start() && point_backward->get_is_start()) {
      result_type = ScanlineSegmentType::kStartingEdge;
    } else if (!point_forward->get_is_start() && !point_backward->get_is_start()) {
      result_type = ScanlineSegmentType::kEndingEdge;
    } else {
      result_type = ScanlineSegmentType::kTurningEdge;
    }
  } else if (!point_forward->get_is_forward() && point_backward->get_is_forward()) {
    if (point_forward->get_id() != point_backward->get_id()) {
      result_type = ScanlineSegmentType::kMutualSpacing;
    } else {
      result_type = ScanlineSegmentType::kSlefSpacing;
    }
  } else if (point_forward->get_is_forward() && !point_backward->get_is_forward()) {
    result_type = ScanlineSegmentType::kWidth;
  }
  return result_type;
}

/// @brief hash two side ids to one
/// @param id1 one side id
/// @param id2 another side id
/// @return hashed side id
uint64_t DrcEngineScanline::hash2SideIds(int id1, int id2)
{
  std::hash<std::pair<int, int>> hasher;
  // TODO: 碰撞
  return hasher(std::make_pair(id1, id2));
}

/// @brief combine sequence to one enum
/// @tparam T
/// @param sequence
/// @param segment_types
template <typename T>
inline void DrcEngineScanline::combineSequence(T& sequence, std::deque<ScanlineSegmentType>& segment_types)
{
  if (segment_types[1] == ScanlineSegmentType::kMutualSpacing) {
    if ((segment_types[0] == ScanlineSegmentType::kStartingEdge && segment_types[2] == ScanlineSegmentType::kWidth)
        || (segment_types[0] == ScanlineSegmentType::kWidth && segment_types[2] == ScanlineSegmentType::kStartingEdge)) {
      sequence = ConditionSequence::SequenceType::kSE_MS_W;
    } else if (segment_types[0] == ScanlineSegmentType::kStartingEdge && segment_types[2] == ScanlineSegmentType::kStartingEdge) {
      sequence = ConditionSequence::SequenceType::kSE_MS_SE;
    } else if ((segment_types[0] == ScanlineSegmentType::kStartingEdge && segment_types[2] == ScanlineSegmentType::kTurningEdge)
               || (segment_types[0] == ScanlineSegmentType::kTurningEdge && segment_types[2] == ScanlineSegmentType::kStartingEdge)) {
      sequence = ConditionSequence::SequenceType::kSE_MS_TE;
    } else if ((segment_types[0] == ScanlineSegmentType::kTurningEdge && segment_types[2] == ScanlineSegmentType::kWidth)
               || (segment_types[0] == ScanlineSegmentType::kWidth && segment_types[2] == ScanlineSegmentType::kTurningEdge)) {
      sequence = ConditionSequence::SequenceType::kTE_MS_W;
    } else if (segment_types[0] == ScanlineSegmentType::kTurningEdge && segment_types[2] == ScanlineSegmentType::kTurningEdge) {
      sequence = ConditionSequence::SequenceType::kTE_MS_TE;
    } else if ((segment_types[0] == ScanlineSegmentType::kEndingEdge && segment_types[2] == ScanlineSegmentType::kWidth)
               || (segment_types[0] == ScanlineSegmentType::kWidth && segment_types[2] == ScanlineSegmentType::kEndingEdge)) {
      sequence = ConditionSequence::SequenceType::kEE_MS_W;
    } else if (segment_types[0] == ScanlineSegmentType::kEndingEdge && segment_types[2] == ScanlineSegmentType::kEndingEdge) {
      sequence = ConditionSequence::SequenceType::kEE_MS_EE;
    } else if ((segment_types[0] == ScanlineSegmentType::kEndingEdge && segment_types[2] == ScanlineSegmentType::kTurningEdge)
               || (segment_types[0] == ScanlineSegmentType::kTurningEdge && segment_types[2] == ScanlineSegmentType::kEndingEdge)) {
      sequence = ConditionSequence::SequenceType::kEE_MS_TE;
    }
  }
}

/// @brief scanline status updated, process current status
/// @param status scanline status
void DrcEngineScanline::processScanlineStatus(ScanlineStatus& status)
{
  std::deque<ScanlinePoint*> activate_points{nullptr, nullptr, nullptr};
  std::deque<ScanlineSegmentType> activate_types{ScanlineSegmentType::kNone, ScanlineSegmentType::kNone, ScanlineSegmentType::kNone};
  auto scanline_status_it = status.insert_begin;
  if (scanline_status_it != status.status_points.end()) {
    activate_points.push_back(*scanline_status_it);
  }

  while (scanline_status_it != status.insert_end) {
    ScanlinePoint* point_backward = *scanline_status_it;
    if (++scanline_status_it != status.insert_end) {
      ScanlinePoint* point_forward = *scanline_status_it;

      // get current segment type
      auto type = judgeSegmentType(status, point_forward, point_backward);

      // refresh active segments
      activate_points.pop_front();
      activate_types.pop_front();
      activate_points.push_back(point_forward);
      activate_types.push_back(type);

      ConditionSequence::SequenceType sequence = ConditionSequence::SequenceType::kNone;
      uint64_t recognize_code = 0;

      // deliver single edge
      bool is_deliver_single_edge = false;
      if (activate_types[2] == ScanlineSegmentType::kStartingEdge) {
        is_deliver_single_edge = true;
        sequence = ConditionSequence::SequenceType::kSE;
      } else if (activate_types[2] == ScanlineSegmentType::kEndingEdge) {
        is_deliver_single_edge = true;
        sequence = ConditionSequence::SequenceType::kEE;
      } else if (activate_types[2] == ScanlineSegmentType::kTurningEdge) {
        is_deliver_single_edge = true;
        sequence = ConditionSequence::SequenceType::kTE;
      }

      if (is_deliver_single_edge) {
        recognize_code = hash2SideIds(point_forward->get_side_id(), point_backward->get_side_id());
        if (_condition_manager->isSequenceNeedDeliver(_preprocess->get_layer(), recognize_code, sequence)) {
          std::vector<DrcBasicPoint*> point_list{point_forward->get_point(), point_backward->get_point()};

          _condition_manager->recordRegion(_preprocess->get_layer(), recognize_code, sequence, point_list, status.current_bucket_coord,
                                           status.direction == ScanlineTravelDirection::kHorizontal);
        }
      }

      // deliver spacing
      if (activate_types[1] == ScanlineSegmentType::kMutualSpacing) {
        recognize_code = hash2SideIds(activate_points[1]->get_side_id(), activate_points[2]->get_side_id());

        combineSequence(sequence, activate_types);
      }

      // put sequence to condition manager
      if (_condition_manager->isSequenceNeedDeliver(_preprocess->get_layer(), recognize_code, sequence)) {
        std::vector<DrcBasicPoint*> point_list(activate_points.size(), nullptr);
        for (int i = 0; i < (int) activate_points.size(); ++i) {
          point_list[i] = activate_points[i] ? activate_points[i]->get_point() : nullptr;
        }

        _condition_manager->recordRegion(_preprocess->get_layer(), recognize_code, sequence, point_list, status.current_bucket_coord,
                                         status.direction == ScanlineTravelDirection::kHorizontal);
      }
    }
  }
}

/// @brief remove ending points
/// @param status scanline status
void DrcEngineScanline::removeEndingPoints(ScanlineStatus& status)
{
  status.status_points.erase(
      std::remove_if(status.status_points.begin(), status.status_points.end(), [](ScanlinePoint* point) { return !point->get_is_start(); }),
      status.status_points.end());
}

}  // namespace idrc