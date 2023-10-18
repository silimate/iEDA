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
 * @file TreeBuilderTest.cc
 * @author Dawn Li (dawnli619215645@gmail.com)
 */
#include <vector>

#include "TreeBuilderAux.hh"
#include "gtest/gtest.h"

namespace {
class TreeBuilderTest : public testing::Test
{
  void SetUp()
  {
    char config[] = "TreeBuilderTest";
    char* argv[] = {config};
    Log::init(argv);
  }
  void TearDown() { Log::end(); }
};

TEST_F(TreeBuilderTest, GeomTest)
{
  using icts::bst::GeomCalc;
  using icts::bst::Pt;
  std::vector<Pt> poly = {Pt(0, 0), Pt(0, 0.5), Pt(0, 1), Pt(1, 1), Pt(1, 0)};
  GeomCalc::convexHull(poly);
  EXPECT_EQ(poly.size(), 4);

  using icts::BalanceClustering;
  using icts::Point;
  std::vector<Point> poly_t = {Point(462000, 649000), Point(701000, 649000), Point(771000, 579000), Point(771000, 504000),
                               Point(658000, 391000), Point(477000, 391000), Point(372000, 496000), Point(372000, 559000)};
  BalanceClustering::convexHull(poly_t);
  EXPECT_EQ(poly_t.size(), 8);
}

TEST_F(TreeBuilderTest, LocalLegalizationTest)
{
  auto* load1 = TreeBuilder::genBufInst("load1", Point(2606905, 3009850));
  TreeBuilder::localPlace({load1->get_load_pin()});
  LOG_INFO << "load1 location: " << load1->get_location();
}

TEST_F(TreeBuilderTest, SimpleTreeBuilderTest)
{
  TreeBuilderAux tree_builder("/home/liweiguo/project/iEDA/scripts/salsa20/iEDA_config/db_default_config.json",
                              "/home/liweiguo/project/iEDA/scripts/salsa20/iEDA_config/cts_default_config.json");
  double skew_bound = 0.08;
  tree_builder.runFixedTest(skew_bound);
}

TEST_F(TreeBuilderTest, RegressionTreeBuilderTest)
{
  TreeBuilderAux tree_builder("/home/liweiguo/project/iEDA/scripts/salsa20/iEDA_config/db_default_config.json",
                              "/home/liweiguo/project/iEDA/scripts/salsa20/iEDA_config/cts_default_config.json");
  std::vector<double> skew_bound_list = {0.08, 0.01, 0.005};
  size_t case_num = 500;
  // design DB unit is 2000
  EnvInfo env_info{50000, 200000, 50000, 200000, 20, 40};
  std::ranges::for_each(skew_bound_list, [&](const double& skew_bound) {
    auto data_set = tree_builder.runRegressTest(env_info, case_num, skew_bound);

    auto suffix = "skew_" + std::to_string(skew_bound);
    // drop "0" in the suffix end
    while (suffix.back() == '0') {
      suffix.pop_back();
    }

    auto dir = CTSAPIInst.get_config()->get_sta_workspace() + "/file/" + suffix;
    auto method_list = {TreeBuilder::funcName(TreeBuilder::fluteTree), TreeBuilder::funcName(TreeBuilder::shallowLightTree),
                        TreeBuilder::funcName(TreeBuilder::boundSkewTree), TreeBuilder::funcName(TreeBuilder::bstSaltTree),
                        TreeBuilder::funcName(TreeBuilder::beatTree)};
    auto topo_type_list = {
        TopoTypeToString(TopoType::kGreedyDist),
        TopoTypeToString(TopoType::kGreedyMerge),
        TopoTypeToString(TopoType::kBiCluster),
        TopoTypeToString(TopoType::kBiPartition),
    };
    // all data
    data_set.writeCSV(method_list, topo_type_list, dir, "regression_" + suffix + ".csv");

    // relative compare
    std::ranges::for_each(method_list, [&](const auto& target_method) { data_set.writeReduceCSV(target_method, dir, suffix); });
  });
}

}  // namespace