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

#include "TAModelStat.hpp"
#include "TANet.hpp"
#include "TAPanel.hpp"
#include "TAPanelId.hpp"

namespace irt {

class TAModel
{
 public:
  TAModel() = default;
  ~TAModel() = default;
  // getter
  std::vector<std::vector<TAPanel>>& get_layer_panel_list() { return _layer_panel_list; }
  std::vector<TANet>& get_ta_net_list() { return _ta_net_list; }
  std::vector<std::vector<TAPanelId>>& get_ta_panel_id_list_list() { return _ta_panel_id_list_list; }
  TAModelStat& get_ta_model_stat() { return _ta_model_stat; }
  irt_int get_curr_iter() { return _curr_iter; }
  // setter
  void set_layer_panel_list(const std::vector<std::vector<TAPanel>>& layer_panel_list) { _layer_panel_list = layer_panel_list; }
  void set_ta_net_list(const std::vector<TANet>& ta_net_list) { _ta_net_list = ta_net_list; }
  void set_ta_panel_id_list_list(const std::vector<std::vector<TAPanelId>>& ta_panel_id_list_list)
  {
    _ta_panel_id_list_list = ta_panel_id_list_list;
  }
  void set_ta_model_stat(const TAModelStat& ta_model_stat) { _ta_model_stat = ta_model_stat; }
  void set_curr_iter(const irt_int curr_iter) { _curr_iter = curr_iter; }

 private:
  std::vector<std::vector<TAPanel>> _layer_panel_list;
  std::vector<TANet> _ta_net_list;
  std::vector<std::vector<TAPanelId>> _ta_panel_id_list_list;
  TAModelStat _ta_model_stat;
  irt_int _curr_iter = -1;
};

}  // namespace irt
