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
 * @project		iDB
 * @file		IdbGroup.h
 * @date		25/05/2021
 * @version		0.1
* @description


    Defines groups in a design.
 *
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "IdbGroup.h"

#include <algorithm>

using namespace std;
namespace idb {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IdbGroup::IdbGroup(string group_name)
{
  _group_name = group_name;
  _region = nullptr;
  _instance_list = new IdbInstanceList();
}

IdbGroup::~IdbGroup()
{
  if (_instance_list) {
    delete _instance_list;
    _instance_list = nullptr;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IdbGroupList::IdbGroupList()
{
  _num = 0;
}

IdbGroupList::~IdbGroupList()
{
  reset();
}

IdbGroup* IdbGroupList::add_group(string name)
{
  IdbGroup* group = new IdbGroup(name);

  _group_list.emplace_back(group);
  _num++;

  return group;
}

void IdbGroupList::reset()
{
  for (auto& group : _group_list) {
    if (group != nullptr) {
      delete group;
      group = nullptr;
    }
  }

  _group_list.clear();
  std::vector<IdbGroup*>().swap(_group_list);
}

}  // namespace idb
