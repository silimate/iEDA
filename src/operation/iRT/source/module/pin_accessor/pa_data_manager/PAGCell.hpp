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

#include "PASourceType.hpp"
#include "RegionQuery.hpp"
#include "SpaceRegion.hpp"

namespace irt {

class PAGCell : public SpaceRegion
{
 public:
  PAGCell() = default;
  ~PAGCell() = default;
  // getter
  std::map<PASourceType, RegionQuery*>& get_source_region_query_map() { return _source_region_query_map; }
  // setter
  void set_source_region_query_map(const std::map<PASourceType, RegionQuery*>& source_region_query_map)
  {
    _source_region_query_map = source_region_query_map;
  }
  // function

 private:
  std::map<PASourceType, RegionQuery*> _source_region_query_map;
};

}  // namespace irt
