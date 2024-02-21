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

#include <any>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../../../database/interaction/RT_DRC/BaseRegion.hpp"
#include "../../../database/interaction/RT_DRC/BaseShape.hpp"
#include "../../../database/interaction/RT_DRC/BaseViolationInfo.hpp"
#include "../../../database/interaction/RT_DRC/DRCCheckType.hpp"
#include "../../../database/interaction/ids.hpp"

namespace irt {

#define RTAPI_INST (irt::RTAPI::getInst())

class RTAPI
{
 public:
  static RTAPI& getInst();
  static void destroyInst();

  // RT
  void initRT(std::map<std::string, std::any> config_map);
  void runRT();
  void destroyRT();

  // EGR
  void runEGR(std::map<std::string, std::any> config_map);

  // EVAL
  eval::TileGrid* getCongestonMap(std::map<std::string, std::any> config_map);
  std::vector<double> getWireLengthAndViaNum(std::map<std::string, std::any> config_map);

  // DRC
  // env_shape_list : blockage obs pin_shape
  // net_idb_segment_map : wire via patch
  std::vector<Violation> getViolationList(std::vector<idb::IdbLayerShape*>& env_shape_list,
                                          std::map<int32_t, std::vector<idb::IdbLayerShape*>>& net_pin_shape_map,
                                          std::map<int32_t, std::vector<idb::IdbRegularWireSegment*>>& net_result_map);

  // STA
  void reportGRTiming();
  void reportDRTiming();

  // other
  void clearDef();

 private:
  static RTAPI* _rt_api_instance;

  RTAPI() = default;
  RTAPI(const RTAPI& other) = delete;
  RTAPI(RTAPI&& other) = delete;
  ~RTAPI() = default;
  RTAPI& operator=(const RTAPI& other) = delete;
  RTAPI& operator=(RTAPI&& other) = delete;
  // function
};

}  // namespace irt
