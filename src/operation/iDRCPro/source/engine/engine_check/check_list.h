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

#include "check_item.h"
#include "condition.h"

namespace idrc {

class CheckList
{
 public:
  CheckList(Condition* condition) : _condition(condition) {}
  ~CheckList()
  {
    for (auto& check_item : _check_list) {
      delete check_item;
    }
  }

  std::vector<CheckItem*>& get_check_list() { return _check_list; }

  void add_check_item(CheckItem* check_item) { _check_list.push_back(check_item); }

  void apply_condition_detail()  // TODO: parallel
  {
    for (auto& check_item : _check_list) {
      _condition->get_detail()->apply(check_item);
    }
  }

 private:
  Condition* _condition;
  std::vector<CheckItem*> _check_list;
};

}  // namespace idrc