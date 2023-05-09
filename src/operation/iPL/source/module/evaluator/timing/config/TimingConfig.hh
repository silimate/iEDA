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
/*
 * @Author: S.J Chen
 * @Date: 2022-04-11 11:25:45
 * @LastEditTime: 2022-04-11 11:47:39
 * @LastEditors: S.J Chen
 * @Description:
 * @FilePath: /iEDA/src/iPL/src/evaluator/timing/config/TimingConfig.hh
 * Contact : https://github.com/sjchanson
 */

#ifndef IPL_EVALUATOR_TIMING_CONFIG_H
#define IPL_EVALUATOR_TIMING_CONFIG_H

#include <string>
#include <vector>

namespace ipl {

class TimingConfig
{
 public:
  TimingConfig()                    = default;
  TimingConfig(const TimingConfig&) = default;
  TimingConfig(TimingConfig&&)      = default;
  ~TimingConfig()                   = default;

  TimingConfig& operator=(const TimingConfig&) = default;
  TimingConfig& operator=(TimingConfig&&) = default;

  // getter.
  std::string              get_sta_workspace_path() const { return _sta_workspace_path; }
  std::string              get_sdc_file_path() const { return _sdc_file_path; }
  std::vector<std::string> get_lib_file_path_list() const { return _lib_file_path_list; }

  // setter.
  void set_sta_workspace_path(std::string path) { _sta_workspace_path = std::move(path); }
  void set_sdc_file_path(std::string path) { _sdc_file_path = std::move(path); }
  void add_lib_file_path(std::string path) { _lib_file_path_list.push_back(std::move(path)); }

 private:
  std::string              _sta_workspace_path;
  std::string              _sdc_file_path;
  std::vector<std::string> _lib_file_path_list;
};

}  // namespace ipl

#endif