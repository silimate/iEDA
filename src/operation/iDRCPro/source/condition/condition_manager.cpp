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

#include "condition_manager.h"

#include "engine_layout.h"
#include "idm.h"

namespace idrc {

void DrcConditionManager::addViolation(ieda_solver::GeometryRect& rect, std::string layer, ViolationEnumType type, std::set<int> net_id)
{
#ifdef _PARALLEL_
#pragma omp single
#endif
  {
    _violation_manager->addViolation(ieda_solver::lowLeftX(rect), ieda_solver::lowLeftY(rect), ieda_solver::upRightX(rect),
                                     ieda_solver::upRightY(rect), type, net_id, layer);
  }
}

}  // namespace idrc