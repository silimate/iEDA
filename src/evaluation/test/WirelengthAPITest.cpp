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
#include <string>

#include "Config.hpp"
#include "EvalAPI.hpp"
#include "EvalLog.hpp"
#include "gtest/gtest.h"
#include "manager.hpp"
#include "usage/usage.hh"

namespace eval {

class WirelengthAPITest : public testing::Test
{
  void SetUp()
  {
    char config[] = "json_test";
    char* argv[] = {config};
    Log::init(argv);
  }
  void TearDown() final { Log::end(); }
};

TEST_F(WirelengthAPITest, sample)
{
  std::string json_file = "/home/yhqiu/irefactor/src/platform/evaluator/source/config/ysyx_eval.json";
  Config* config = Config::getOrCreateConfig(json_file);
  Manager::initInst(config);
  auto wirelength_evaluator = Manager::getInst().getWirelengthEval();
  auto& wl_net_list = wirelength_evaluator->get_net_list();

  EvalAPI& eval_api = EvalAPI::initInst();
  int64_t wl;
  // Test Result: PASS
  // wl = eval_api.evalTotalWL("kHPWL", wl_net_list);
  // wl = eval_api.evalTotalWL("kHTree", wl_net_list);
  // wl = eval_api.evalTotalWL("kVTree", wl_net_list);
  // wl = eval_api.evalTotalWL("kClique", wl_net_list);
  // wl = eval_api.evalTotalWL("kStar", wl_net_list);
  // wl = eval_api.evalTotalWL("kB2B", wl_net_list);
  // wl = eval_api.evalTotalWL("kFlute", wl_net_list);
  // wl = eval_api.evalTotalWL("kDR", wl_net_list);
  // wl = eval_api.evalTotalWL("None", wl_net_list);

  // Test Result: PASS
  // wl = eval_api.evalOneNetWL("kHPWL", wl_net_list[0]);
  // wl = eval_api.evalOneNetWL("kHTree", wl_net_list[0]);
  // wl = eval_api.evalOneNetWL("kVTree", wl_net_list[0]);
  // wl = eval_api.evalOneNetWL("kClique", wl_net_list[0]);
  // wl = eval_api.evalOneNetWL("kStar", wl_net_list[0]);
  // wl = eval_api.evalOneNetWL("kB2B", wl_net_list[0]);
  // wl = eval_api.evalOneNetWL("kFlute", wl_net_list[0]);
  // wl = eval_api.evalOneNetWL("kDR", wl_net_list[0]);
  // wl = eval_api.evalOneNetWL("None", wl_net_list[0]);

  // Test Result: PASS
  // wl = eval_api.evalDriver2LoadWL(wl_net_list[0], "I");
  // wl = eval_api.evalDriver2LoadWL(wl_net_list[0], "I");
  // wl = eval_api.evalDriver2LoadWL(wl_net_list[0], "I");
  // wl = eval_api.evalDriver2LoadWL(wl_net_list[0], "I");
  // wl = eval_api.evalDriver2LoadWL(wl_net_list[0], "I");
  // wl = eval_api.evalDriver2LoadWL(wl_net_list[0], "I");
  // wl = eval_api.evalDriver2LoadWL(wl_net_list[0], "I");
  // wl = eval_api.evalDriver2LoadWL(wl_net_list[0], "I");
  // wl = eval_api.evalDriver2LoadWL(wl_net_list[0], "I");
  LOG_INFO << "wl :" << wl;
  EvalAPI::destroyInst();
  Manager::destroyInst();
}

}  // namespace eval