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

#include "idrc_dm.h"
#include "idrc_engine_manager.h"
#include "idrc_data.h"

namespace idrc {
class DrcEngine
{
 public:
  DrcEngine(DrcDataManager* data_manager);
  ~DrcEngine();

  DrcEngineManager* get_engine_manager() { return _engine_manager; }
  void initEngine(DrcCheckerType checker_type = DrcCheckerType::kRT);

  /// scanline operator
  void initEngineScanline();

 private:
  DrcDataManager* _data_manager = nullptr;
  DrcEngineManager* _engine_manager = nullptr;

  void initEngineGeometryData();
  void initEngineDef();
};

}  // namespace idrc