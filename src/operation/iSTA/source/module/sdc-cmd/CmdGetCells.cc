// ***************************************************************************************
// Copyright (c) 2023-2025 Peng Cheng Laboratory
// Copyright (c) 2023-2025 Institute of Computing Technology, Chinese Academy of
// Sciences Copyright (c) 2023-2025 Beijing Institute of Open Source Chip
//
// iEDA is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan
// PSL v2. You may obtain a copy of Mulan PSL v2 at:
// http://license.coscl.org.cn/MulanPSL2
//
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
//
// See the Mulan PSL v2 for more details.
// ***************************************************************************************
/**
 * @file CmdGetLibs.cc
 * @author simin tao (taosm@pcl.ac.cn)
 * @brief The sdc cmd of get_cells
 * @version 0.1
 * @date 2024-06-24
 */
#include "Cmd.hh"
#include "sdc/SdcCollection.hh"
#include "wildcards/single_include/wildcards.hpp"

namespace ista {

CmdGetCells::CmdGetCells(const char* cmd_name) : TclCmd(cmd_name) {
  auto* patterns_arg = new TclStringListOption("patterns", 1, {});
  addOption(patterns_arg);
}

unsigned CmdGetCells::check() { return 1; }

/**
 * @brief execute the get_libs.
 *
 * @return unsigned
 */
unsigned CmdGetCells::exec() {
  Sta* ista = Sta::getOrCreateSta();
  auto& the_constrain = ista->get_constrains();

  auto* cell_pattern_arg = getOptionOrArg("patterns");
  auto cell_patterns = cell_pattern_arg->getStringList();

  std::vector<SdcCollectionObj> obj_list;

  Netlist* design_nl = ista->get_netlist();
  auto& instances = design_nl->get_instances();
  for (auto& instance : instances) {
    for (auto& pattern : cell_patterns) {
      // match lib name all str.
      std::string_view pattern_str = pattern;
      std::string instance_name = instance.getFullName();
      if (wildcards::match(instance_name, pattern_str)) {
        obj_list.emplace_back(&instance);
      }
    }
  }

  auto* sdc_collection = new SdcCollection(SdcCollection::CollectionType::kPin,
                                           std::move(obj_list));

  the_constrain->addSdcCollection(sdc_collection);

  char* result = TclEncodeResult::encode(sdc_collection);
  ScriptEngine::getOrCreateInstance()->setResult(result);

  return 1;
}

}  // namespace ista