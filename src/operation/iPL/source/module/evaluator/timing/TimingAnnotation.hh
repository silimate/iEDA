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

#ifndef IPL_EVALUATOR_TIMING_H
#define IPL_EVALUATOR_TIMING_H

#include <map>
#include <vector>

#include "SteinerWirelength.hh"
#include "TopologyManager.hh"

namespace ipl {

class TimingAnnotation
{
 public:
  TimingAnnotation() = delete;
  explicit TimingAnnotation(TopologyManager* topology_manager);
  TimingAnnotation(const TimingAnnotation&) = delete;
  TimingAnnotation(TimingAnnotation&&) = delete;
  ~TimingAnnotation() = default;

  TimingAnnotation& operator=(const TimingAnnotation&) = delete;
  TimingAnnotation& operator=(TimingAnnotation&&) = delete;

  // getter.
  float get_early_wns();
  float get_late_wns();
  float get_early_tns();
  float get_late_tns();

  float getOutNodeRes(Node* out_node);   // TODO
  float getAvgWireResPerUnitLength();    // TODO
  float getAvgWireCapPerUnitLength();    // TODO
  float getNodeInputCap(Node* in_node);  // TODO

  float get_node_early_slack(int32_t node_id);
  float get_node_late_slack(int32_t node_id);
  float get_node_early_arrival_time(int32_t node_id);
  float get_node_late_arrival_time(int32_t node_id);
  float get_node_early_required_time(int32_t node_id);
  float get_node_late_required_time(int32_t node_id);

  float get_node_criticality(Node* node);
  float get_group_criticality(Group* group);
  float get_network_criticality(NetWork* network);

  // only for fliplfop.
  Node* get_clock_node(Group* flipflop);
  Node* get_data_node(Group* fliplfop);

  const std::vector<Node*>& get_topo_order_node_list() const { return _topo_order_node_list; }
  const std::vector<NetWork*>& get_topo_order_network_list() const { return _topo_order_net_list; }
  const std::vector<Group*>& get_topo_order_group_list() const { return _topo_order_group_list; }

  // function.
  void updateSTATimingFull();
  void updateSTATimingIncremental();
  void reportCurrentTiming();

  void updateCriticalityAndCentralityFull();
  void updateCriticalityAndCentralityIncremental(const std::vector<NetWork*>& network_list);

  std::vector<Group*> obtainEarlyViolatedGroupListByTopoOrder();
  std::vector<Group*> obtainLateViolatedGroupListByTopoOrder();

 private:
  TopologyManager* _topology_manager;
  SteinerWirelength* _steiner_wirelength;

  float _max_centrality;

  // Topology order
  std::vector<Node*> _topo_order_node_list;
  std::vector<NetWork*> _topo_order_net_list;
  std::vector<Group*> _topo_order_group_list;

  void init();
  void updateCriticalityAndCentrality(NetWork* network);
};
inline TimingAnnotation::TimingAnnotation(TopologyManager* topology_manager) : _topology_manager(topology_manager), _max_centrality(0.0f)
{
  init();
}

}  // namespace ipl

#endif