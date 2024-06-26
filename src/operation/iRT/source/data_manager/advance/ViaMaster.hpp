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

#include "LayerRect.hpp"

namespace irt {

class ViaMasterIdx
{
 public:
  ViaMasterIdx() = default;
  ~ViaMasterIdx() = default;
  // getter
  int32_t get_below_layer_idx() const { return _below_layer_idx; }
  int32_t get_via_idx() const { return _via_idx; }
  // setter
  void set_below_layer_idx(const int32_t below_layer_idx) { _below_layer_idx = below_layer_idx; }
  void set_via_idx(const int32_t via_idx) { _via_idx = via_idx; }
  // function

 private:
  int32_t _below_layer_idx = -1;
  int32_t _via_idx = -1;
};

class ViaMaster
{
 public:
  ViaMaster() = default;
  ~ViaMaster() = default;
  // getter
  ViaMasterIdx& get_via_master_idx() { return _via_master_idx; }
  std::string& get_via_name() { return _via_name; }
  LayerRect& get_above_enclosure() { return _above_enclosure; }
  Direction get_above_direction() const { return _above_direction; }
  LayerRect& get_below_enclosure() { return _below_enclosure; }
  Direction get_below_direction() const { return _below_direction; }
  std::vector<PlanarRect>& get_cut_shape_list() { return _cut_shape_list; }
  int32_t get_cut_layer_idx() const { return _cut_layer_idx; }
  // setter
  void set_via_master_idx(const ViaMasterIdx& via_master_idx) { _via_master_idx = via_master_idx; }
  void set_via_master_idx(const int32_t below_layer_idx, const int32_t via_idx)
  {
    _via_master_idx.set_below_layer_idx(below_layer_idx);
    _via_master_idx.set_via_idx(via_idx);
  }
  void set_via_name(const std::string& via_name) { _via_name = via_name; }
  void set_above_enclosure(const LayerRect& above_enclosure) { _above_enclosure = above_enclosure; }
  void set_above_direction(const Direction& above_direction) { _above_direction = above_direction; }
  void set_below_enclosure(const LayerRect& below_enclosure) { _below_enclosure = below_enclosure; }
  void set_below_direction(const Direction& below_direction) { _below_direction = below_direction; }
  void set_cut_shape_list(const std::vector<PlanarRect>& cut_shape_list) { _cut_shape_list = cut_shape_list; }
  void set_cut_layer_idx(const int32_t cut_layer_idx) { _cut_layer_idx = cut_layer_idx; }
  // function

 private:
  ViaMasterIdx _via_master_idx;
  std::string _via_name;
  LayerRect _above_enclosure;
  Direction _above_direction = Direction::kNone;
  LayerRect _below_enclosure;
  Direction _below_direction = Direction::kNone;
  std::vector<PlanarRect> _cut_shape_list;
  int32_t _cut_layer_idx;
};

}  // namespace irt
