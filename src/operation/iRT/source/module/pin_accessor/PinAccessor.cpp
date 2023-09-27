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
#include "PinAccessor.hpp"

#include <sstream>

#include "GDSPlotter.hpp"
#include "GPGDS.hpp"
#include "PAModel.hpp"
#include "PANet.hpp"

namespace irt {

// public

void PinAccessor::initInst()
{
  if (_pa_instance == nullptr) {
    _pa_instance = new PinAccessor();
  }
}

PinAccessor& PinAccessor::getInst()
{
  if (_pa_instance == nullptr) {
    LOG_INST.error(Loc::current(), "The instance not initialized!");
  }
  return *_pa_instance;
}

void PinAccessor::destroyInst()
{
  if (_pa_instance != nullptr) {
    delete _pa_instance;
    _pa_instance = nullptr;
  }
}

// function

void PinAccessor::access(std::vector<Net>& net_list)
{
  Monitor monitor;

  accessNetList(net_list);

  LOG_INST.info(Loc::current(), "The ", GetStageName()(Stage::kPinAccessor), " completed!", monitor.getStatsInfo());
}

// private

PinAccessor* PinAccessor::_pa_instance = nullptr;

void PinAccessor::accessNetList(std::vector<Net>& net_list)
{
  PAModel pa_model = init(net_list);
  iterative(pa_model);
  update(pa_model);
}

#if 1  // init

PAModel PinAccessor::init(std::vector<Net>& net_list)
{
  PAModel pa_model = initPAModel(net_list);
  buildPAModel(pa_model);
  checkPAModel(pa_model);
  return pa_model;
}

PAModel PinAccessor::initPAModel(std::vector<Net>& net_list)
{
  ScaleAxis& gcell_axis = DM_INST.getDatabase().get_gcell_axis();
  Die& die = DM_INST.getDatabase().get_die();
  std::vector<RoutingLayer>& routing_layer_list = DM_INST.getDatabase().get_routing_layer_list();

  PAModel pa_model;
  GridMap<PAGCell>& pa_gcell_map = pa_model.get_pa_gcell_map();
  pa_gcell_map.init(die.getXSize(), die.getYSize());
  for (irt_int x = 0; x < die.getXSize(); x++) {
    for (irt_int y = 0; y < die.getYSize(); y++) {
      PAGCell& pa_gcell = pa_gcell_map[x][y];
      pa_gcell.set_base_region(RTUtil::getRealRect(x, y, gcell_axis));
      pa_gcell.set_top_layer_idx(routing_layer_list.back().get_layer_idx());
      pa_gcell.set_bottom_layer_idx(routing_layer_list.front().get_layer_idx());
    }
  }
  pa_model.set_pa_net_list(convertToPANetList(net_list));

  return pa_model;
}

std::vector<PANet> PinAccessor::convertToPANetList(std::vector<Net>& net_list)
{
  std::vector<PANet> pa_net_list;
  pa_net_list.reserve(net_list.size());
  for (Net& net : net_list) {
    pa_net_list.emplace_back(convertToPANet(net));
  }
  return pa_net_list;
}

PANet PinAccessor::convertToPANet(Net& net)
{
  PANet pa_net;
  pa_net.set_origin_net(&net);
  pa_net.set_net_idx(net.get_net_idx());
  pa_net.set_connect_type(net.get_connect_type());
  for (Pin& pin : net.get_pin_list()) {
    pa_net.get_pa_pin_list().push_back(PAPin(pin));
  }
  pa_net.set_pa_driving_pin(PAPin(net.get_driving_pin()));
  pa_net.set_bounding_box(net.get_bounding_box());
  return pa_net;
}

void PinAccessor::buildPAModel(PAModel& pa_model)
{
  updateBlockageMap(pa_model);
  updateNetShapeMap(pa_model);
}

void PinAccessor::updateBlockageMap(PAModel& pa_model)
{
  std::vector<Blockage>& routing_blockage_list = DM_INST.getDatabase().get_routing_blockage_list();
  std::vector<Blockage>& cut_blockage_list = DM_INST.getDatabase().get_cut_blockage_list();

  for (Blockage& routing_blockage : routing_blockage_list) {
    LayerRect blockage_real_rect(routing_blockage.get_real_rect(), routing_blockage.get_layer_idx());
    addRectToEnv(pa_model, PASourceType::kBlockage, DRCRect(-1, blockage_real_rect, true));
  }
  for (Blockage& cut_blockage : cut_blockage_list) {
    LayerRect blockage_real_rect(cut_blockage.get_real_rect(), cut_blockage.get_layer_idx());
    addRectToEnv(pa_model, PASourceType::kBlockage, DRCRect(-1, blockage_real_rect, false));
  }
}

void PinAccessor::addRectToEnv(PAModel& pa_model, PASourceType pa_source_type, DRCRect drc_rect)
{
  ScaleAxis& gcell_axis = DM_INST.getDatabase().get_gcell_axis();
  EXTPlanarRect& die = DM_INST.getDatabase().get_die();

  GridMap<PAGCell>& pa_gcell_map = pa_model.get_pa_gcell_map();

  for (const LayerRect& max_scope_real_rect : DC_INST.getMaxScope(drc_rect)) {
    LayerRect max_scope_regular_rect = RTUtil::getRegularRect(max_scope_real_rect, die.get_real_rect());
    PlanarRect max_scope_grid_rect = RTUtil::getClosedGridRect(max_scope_regular_rect, gcell_axis);
    for (irt_int x = max_scope_grid_rect.get_lb_x(); x <= max_scope_grid_rect.get_rt_x(); x++) {
      for (irt_int y = max_scope_grid_rect.get_lb_y(); y <= max_scope_grid_rect.get_rt_y(); y++) {
        PAGCell& pa_gcell = pa_gcell_map[x][y];
        DC_INST.updateRectList(pa_gcell.getRegionQuery(pa_source_type), ChangeType::kAdd, drc_rect);
      }
    }
  }
}

void PinAccessor::updateNetShapeMap(PAModel& pa_model)
{
  for (PANet& pa_net : pa_model.get_pa_net_list()) {
    for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
      for (EXTLayerRect& routing_shape : pa_pin.get_routing_shape_list()) {
        LayerRect shape_real_rect(routing_shape.get_real_rect(), routing_shape.get_layer_idx());
        addRectToEnv(pa_model, PASourceType::kNetShape, DRCRect(pa_net.get_net_idx(), shape_real_rect, true));
      }
      for (EXTLayerRect& cut_shape : pa_pin.get_cut_shape_list()) {
        LayerRect shape_real_rect(cut_shape.get_real_rect(), cut_shape.get_layer_idx());
        addRectToEnv(pa_model, PASourceType::kNetShape, DRCRect(pa_net.get_net_idx(), shape_real_rect, false));
      }
    }
  }
}

void PinAccessor::checkPAModel(PAModel& pa_model)
{
}

#endif

#if 1  // iterative

void PinAccessor::iterative(PAModel& pa_model)
{
  irt_int pa_max_iter_num = DM_INST.getConfig().pa_max_iter_num;

  for (irt_int iter = 1; iter <= pa_max_iter_num; iter++) {
    Monitor iter_monitor;
    LOG_INST.info(Loc::current(), "****** Start Iteration(", iter, "/", pa_max_iter_num, ") ******");
    pa_model.set_curr_iter(iter);
    accessPAModel(pa_model);
    processPAModel(pa_model);
    countPAModel(pa_model);
    reportPAModel(pa_model);
    LOG_INST.info(Loc::current(), "****** End Iteration(", iter, "/", pa_max_iter_num, ")", iter_monitor.getStatsInfo(), " ******");
    if (stopPAModel(pa_model)) {
      LOG_INST.info(Loc::current(), "****** Reached the stopping condition, ending the iteration prematurely! ******");
      pa_model.set_curr_iter(-1);
      break;
    }
  }
}

void PinAccessor::accessPAModel(PAModel& pa_model)
{
  accessPANetList(pa_model);
  updateNetCandidateViaMap(pa_model);
  filterPANetList(pa_model);
}

void PinAccessor::accessPANetList(PAModel& pa_model)
{
  Monitor monitor;

  std::vector<PANet>& pa_net_list = pa_model.get_pa_net_list();

  irt_int batch_size = RTUtil::getBatchSize(pa_net_list.size());

  Monitor stage_monitor;
#pragma omp parallel for
  for (size_t i = 0; i < pa_net_list.size(); i++) {
    accessPANet(pa_model, pa_net_list[i]);
    if (omp_get_num_threads() == 1 && (i + 1) % batch_size == 0) {
      LOG_INST.info(Loc::current(), "Accessed ", (i + 1), " nets", stage_monitor.getStatsInfo());
    }
  }
  if (omp_get_num_threads() == 1) {
    LOG_INST.info(Loc::current(), "Accessed ", pa_net_list.size(), " nets", monitor.getStatsInfo());
  }
}

void PinAccessor::accessPANet(PAModel& pa_model, PANet& pa_net)
{
  initAccessPointList(pa_model, pa_net);
  mergeAccessPointList(pa_net);
  updateAccessGridCoord(pa_net);
  updateViaAccessByDRC(pa_model, pa_net);
  updateWireAccessByDRC(pa_model, pa_net);
  updateWeakAccessByDRC(pa_model, pa_net);
  initProtectedPointList(pa_net);
  checkProtectedPointList(pa_net);
}

void PinAccessor::initAccessPointList(PAModel& pa_model, PANet& pa_net)
{
  std::vector<RoutingLayer>& routing_layer_list = DM_INST.getDatabase().get_routing_layer_list();
  irt_int top_routing_layer_idx = DM_INST.getConfig().top_routing_layer_idx;
  irt_int bottom_routing_layer_idx = DM_INST.getConfig().bottom_routing_layer_idx;

  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    std::vector<AccessPoint>& access_point_list = pa_pin.get_access_point_list();
    for (LayerRect& aligned_pin_shape : getLegalPinShapeList(pa_model, pa_net.get_net_idx(), pa_pin)) {
      irt_int lb_x = aligned_pin_shape.get_lb_x();
      irt_int lb_y = aligned_pin_shape.get_lb_y();
      irt_int rt_x = aligned_pin_shape.get_rt_x();
      irt_int rt_y = aligned_pin_shape.get_rt_y();
      irt_int pin_shape_layer_idx = aligned_pin_shape.get_layer_idx();

      // routing layer info
      std::vector<irt_int> layer_idx_list;
      irt_int mid_layer_idx = pin_shape_layer_idx;
      mid_layer_idx = std::min(mid_layer_idx, top_routing_layer_idx);
      mid_layer_idx = std::max(mid_layer_idx, bottom_routing_layer_idx);
      for (irt_int layer_idx : {mid_layer_idx - 1, mid_layer_idx, mid_layer_idx + 1}) {
        if (layer_idx < bottom_routing_layer_idx || top_routing_layer_idx < layer_idx) {
          continue;
        }
        layer_idx_list.push_back(layer_idx);
      }
      // prefer track grid
      for (irt_int i = 0; i < static_cast<irt_int>(layer_idx_list.size()) - 1; i++) {
        RoutingLayer curr_routing_layer = routing_layer_list[layer_idx_list[i]];
        RoutingLayer upper_routing_layer = routing_layer_list[layer_idx_list[i + 1]];
        if (curr_routing_layer.isPreferH()) {
          for (irt_int x : RTUtil::getClosedScaleList(lb_x, rt_x, upper_routing_layer.getPreferTrackGridList())) {
            for (irt_int y : RTUtil::getClosedScaleList(lb_y, rt_y, curr_routing_layer.getPreferTrackGridList())) {
              access_point_list.emplace_back(x, y, pin_shape_layer_idx, AccessPointType::kPrefTrackGrid);
            }
          }
        } else {
          for (irt_int x : RTUtil::getClosedScaleList(lb_x, rt_x, curr_routing_layer.getPreferTrackGridList())) {
            for (irt_int y : RTUtil::getClosedScaleList(lb_y, rt_y, upper_routing_layer.getPreferTrackGridList())) {
              access_point_list.emplace_back(x, y, pin_shape_layer_idx, AccessPointType::kPrefTrackGrid);
            }
          }
        }
      }
      // curr layer track grid
      for (irt_int layer_idx : layer_idx_list) {
        for (irt_int x : RTUtil::getClosedScaleList(lb_x, rt_x, routing_layer_list[layer_idx].getXTrackGridList())) {
          for (irt_int y : RTUtil::getClosedScaleList(lb_y, rt_y, routing_layer_list[layer_idx].getYTrackGridList())) {
            access_point_list.emplace_back(x, y, pin_shape_layer_idx, AccessPointType::kCurrTrackGrid);
          }
        }
      }
      // on track
      irt_int mid_x = (lb_x + rt_x) / 2;
      irt_int mid_y = (lb_y + rt_y) / 2;
      for (irt_int layer_idx : layer_idx_list) {
        for (irt_int x : RTUtil::getClosedScaleList(lb_x, rt_x, routing_layer_list[layer_idx].getXTrackGridList())) {
          for (irt_int y : {lb_y, mid_y, rt_y}) {
            access_point_list.emplace_back(x, y, pin_shape_layer_idx, AccessPointType::kOnTrack);
          }
        }
        for (irt_int y : RTUtil::getClosedScaleList(lb_y, rt_y, routing_layer_list[layer_idx].getYTrackGridList())) {
          for (irt_int x : {lb_x, mid_x, rt_x}) {
            access_point_list.emplace_back(x, y, pin_shape_layer_idx, AccessPointType::kOnTrack);
          }
        }
      }
      // on shape
      for (irt_int x : {lb_x, mid_x, rt_x}) {
        for (irt_int y : {lb_y, mid_y, rt_y}) {
          access_point_list.emplace_back(x, y, pin_shape_layer_idx, AccessPointType::kOnShape);
        }
      }
    }
    if (access_point_list.empty()) {
      LOG_INST.error(Loc::current(), "No access point was generated!");
    }
  }
}

std::vector<LayerRect> PinAccessor::getLegalPinShapeList(PAModel& pa_model, irt_int pa_net_idx, PAPin& pa_pin)
{
#if 0
  irt_int bottom_routing_layer_idx = DM_INST.getConfig().bottom_routing_layer_idx;
  irt_int top_routing_layer_idx = DM_INST.getConfig().top_routing_layer_idx;

  std::map<irt_int, std::vector<EXTLayerRect>> layer_pin_shape_list;
  for (EXTLayerRect& routing_shape : pa_pin.get_routing_shape_list()) {
    layer_pin_shape_list[routing_shape.get_layer_idx()].emplace_back(routing_shape);
  }

  std::vector<LayerRect> legal_rect_list;
  for (auto& [layer_idx, pin_shpae_list] : layer_pin_shape_list) {
    std::vector<LayerRect> curr_legal_rect_list;
    for (irt_int via_below_layer_idx :
         RTUtil::getReservedViaBelowLayerIdxList(layer_idx, bottom_routing_layer_idx, top_routing_layer_idx)) {
      std::vector<PlanarRect> via_legal_rect_list = getViaLegalRectList(pa_model, pa_net_idx, via_below_layer_idx, pin_shpae_list);
      if (via_legal_rect_list.empty()) {
        curr_legal_rect_list.clear();
        break;
      }
      for (PlanarRect via_legal_rect : via_legal_rect_list) {
        curr_legal_rect_list.emplace_back(via_legal_rect, layer_idx);
      }
    }
    legal_rect_list.insert(legal_rect_list.end(), curr_legal_rect_list.begin(), curr_legal_rect_list.end());
  }
  if (!legal_rect_list.empty()) {
    return legal_rect_list;
  }
  for (auto& [layer_idx, pin_shpae_list] : layer_pin_shape_list) {
    for (PlanarRect wire_legal_rect : getWireLegalRectList(pa_model, pa_net_idx, pin_shpae_list)) {
      legal_rect_list.emplace_back(wire_legal_rect, layer_idx);
    }
  }
  if (!legal_rect_list.empty()) {
    return legal_rect_list;
  }
  // LOG_INST.warning(Loc::current(), "The pin ", pa_pin.get_pin_name(), " use all pin shapes as a legal area!");
  for (EXTLayerRect& routing_shape : pa_pin.get_routing_shape_list()) {
    legal_rect_list.emplace_back(routing_shape.get_real_rect(), routing_shape.get_layer_idx());
  }
  return legal_rect_list;
#else
  std::vector<LayerRect> legal_rect_list;
  for (EXTLayerRect& routing_shape : pa_pin.get_routing_shape_list()) {
    legal_rect_list.emplace_back(routing_shape.get_real_rect(), routing_shape.get_layer_idx());
  }
  return legal_rect_list;
#endif
}

std::vector<PlanarRect> PinAccessor::getViaLegalRectList(PAModel& pa_model, irt_int pa_net_idx, irt_int via_below_layer_idx,
                                                         std::vector<EXTLayerRect>& pin_shape_list)
{
  for (EXTLayerRect& pin_shape : pin_shape_list) {
    if (pin_shape_list.front().get_layer_idx() != pin_shape.get_layer_idx()) {
      LOG_INST.error(Loc::current(), "The pin_shape_list is not on the same layer!");
    }
  }
  std::vector<std::vector<ViaMaster>>& layer_via_master_list = DM_INST.getDatabase().get_layer_via_master_list();
  std::vector<RoutingLayer>& routing_layer_list = DM_INST.getDatabase().get_routing_layer_list();

  GridMap<PAGCell>& pa_gcell_map = pa_model.get_pa_gcell_map();

  if (via_below_layer_idx < routing_layer_list.front().get_layer_idx()
      || routing_layer_list.back().get_layer_idx() <= via_below_layer_idx) {
    return {};
  }
  ViaMaster& via_master = layer_via_master_list[via_below_layer_idx].front();

  // pin_shape 通过enclosure往里缩小的的形状
  std::vector<PlanarRect> enclosure_reduced_list;
  {
    irt_int half_x_span = -1;
    irt_int half_y_span = -1;
    if (via_master.get_above_enclosure().get_layer_idx() == pin_shape_list.front().get_layer_idx()) {
      half_x_span = via_master.get_above_enclosure().getXSpan() / 2;
      half_y_span = via_master.get_above_enclosure().getYSpan() / 2;
    } else {
      half_x_span = via_master.get_below_enclosure().getXSpan() / 2;
      half_y_span = via_master.get_below_enclosure().getYSpan() / 2;
    }
    for (EXTLayerRect& pin_shape : pin_shape_list) {
      enclosure_reduced_list.push_back(pin_shape.get_real_rect());
    }
    // 当前层缩小后的结果
    enclosure_reduced_list
        = RTUtil::getClosedReducedRectByBoost(enclosure_reduced_list, half_x_span, half_y_span, half_x_span, half_y_span);
  }
  // pin_shape 原始的形状
  std::vector<PlanarRect> origin_pin_shape_list;
  for (EXTLayerRect& pin_shape : pin_shape_list) {
    origin_pin_shape_list.push_back(pin_shape.get_real_rect());
  }
  // pin_shape 由于routing_obs_shape要被剪裁的形状
  std::vector<PlanarRect> routing_obs_shape_list;
  for (LayerRect enclosure : {via_master.get_above_enclosure(), via_master.get_below_enclosure()}) {
    irt_int half_x_span = enclosure.getXSpan() / 2;
    irt_int half_y_span = enclosure.getYSpan() / 2;

    for (EXTLayerRect& pin_shape : pin_shape_list) {
      for (irt_int x = pin_shape.get_grid_lb_x(); x <= pin_shape.get_grid_rt_x(); x++) {
        for (irt_int y = pin_shape.get_grid_lb_y(); y <= pin_shape.get_grid_rt_y(); y++) {
          PAGCell& pa_gcell = pa_gcell_map[x][y];
          for (PASourceType pa_source_type : {PASourceType::kBlockage, PASourceType::kNetShape}) {
            for (const auto& [curr_net_idx, rect_list] :
                 DC_INST.getLayerNetRectMap(pa_gcell.getRegionQuery(pa_source_type), true)[enclosure.get_layer_idx()]) {
              if (pa_net_idx == curr_net_idx) {
                continue;
              }
              for (const LayerRect& rect : rect_list) {
                for (LayerRect& min_scope_blockage : DC_INST.getMinScope(DRCRect(curr_net_idx, rect, true))) {
                  PlanarRect enlarged_rect
                      = RTUtil::getEnlargedRect(min_scope_blockage, half_x_span, half_y_span, half_x_span, half_y_span);
                  if (!RTUtil::isOpenOverlap(pin_shape.get_real_rect(), enlarged_rect)) {
                    continue;
                  }
                  routing_obs_shape_list.push_back(enlarged_rect);
                }
              }
            }
          }
        }
      }
    }
  }
  // pin_shape 由于cut_obs_shape要被剪裁的形状
  std::vector<PlanarRect> cut_obs_shape_list;
  {
    PlanarRect cut_shape = RTUtil::getBoundingBox(via_master.get_cut_shape_list());
    irt_int half_x_span = cut_shape.getXSpan() / 2;
    irt_int half_y_span = cut_shape.getYSpan() / 2;
    for (EXTLayerRect& pin_shape : pin_shape_list) {
      for (irt_int x = pin_shape.get_grid_lb_x(); x <= pin_shape.get_grid_rt_x(); x++) {
        for (irt_int y = pin_shape.get_grid_lb_y(); y <= pin_shape.get_grid_rt_y(); y++) {
          PAGCell& pa_gcell = pa_gcell_map[x][y];
          for (PASourceType pa_source_type : {PASourceType::kBlockage, PASourceType::kNetShape}) {
            for (const auto& [curr_net_idx, rect_list] :
                 DC_INST.getLayerNetRectMap(pa_gcell.getRegionQuery(pa_source_type), false)[via_master.get_cut_layer_idx()]) {
              if (pa_net_idx == curr_net_idx) {
                continue;
              }
              for (const LayerRect& rect : rect_list) {
                for (LayerRect& min_scope_blockage : DC_INST.getMinScope(DRCRect(curr_net_idx, rect, false))) {
                  PlanarRect enlarged_rect
                      = RTUtil::getEnlargedRect(min_scope_blockage, half_x_span, half_y_span, half_x_span, half_y_span);
                  if (!RTUtil::isOpenOverlap(pin_shape.get_real_rect(), enlarged_rect)) {
                    continue;
                  }
                  cut_obs_shape_list.push_back(enlarged_rect);
                }
              }
            }
          }
        }
      }
    }
  }
  std::vector<PlanarRect> routing_layer_legal_rect_list
      = RTUtil::getClosedCuttingRectListByBoost(origin_pin_shape_list, routing_obs_shape_list);
  std::vector<PlanarRect> cut_layer_legal_rect_list = RTUtil::getClosedCuttingRectListByBoost(origin_pin_shape_list, cut_obs_shape_list);
  std::vector<PlanarRect> via_legal_rect_list = RTUtil::getClosedOverlapByBoost(routing_layer_legal_rect_list, cut_layer_legal_rect_list);
  std::vector<PlanarRect> reduced_legal_rect_list = RTUtil::getClosedOverlapByBoost(via_legal_rect_list, enclosure_reduced_list);
  if (!reduced_legal_rect_list.empty()) {
    via_legal_rect_list = reduced_legal_rect_list;
  }
  return via_legal_rect_list;
}

std::vector<PlanarRect> PinAccessor::getWireLegalRectList(PAModel& pa_model, irt_int pa_net_idx, std::vector<EXTLayerRect>& pin_shape_list)
{
  for (EXTLayerRect& pin_shape : pin_shape_list) {
    if (pin_shape_list.front().get_layer_idx() != pin_shape.get_layer_idx()) {
      LOG_INST.error(Loc::current(), "The pin_shape_list is not on the same layer!");
    }
  }
  std::vector<RoutingLayer>& routing_layer_list = DM_INST.getDatabase().get_routing_layer_list();

  GridMap<PAGCell>& pa_gcell_map = pa_model.get_pa_gcell_map();

  irt_int pin_shape_layer_idx = pin_shape_list.front().get_layer_idx();
  irt_int half_width = routing_layer_list[pin_shape_layer_idx].get_min_width();

  // pin_shape 通过wire往里缩小的的形状
  std::vector<PlanarRect> wire_reduced_list;
  {
    for (EXTLayerRect& pin_shape : pin_shape_list) {
      wire_reduced_list.push_back(pin_shape.get_real_rect());
    }
    // 当前层缩小后的结果
    wire_reduced_list = RTUtil::getClosedReducedRectByBoost(wire_reduced_list, half_width);
  }
  // pin_shape 原始的形状
  std::vector<PlanarRect> origin_pin_shape_list;
  for (EXTLayerRect& pin_shape : pin_shape_list) {
    origin_pin_shape_list.push_back(pin_shape.get_real_rect());
  }
  // pin_shape 由于routing_obs_shape要被剪裁的形状
  std::vector<PlanarRect> routing_obs_shape_list;

  for (EXTLayerRect& pin_shape : pin_shape_list) {
    for (irt_int x = pin_shape.get_grid_lb_x(); x <= pin_shape.get_grid_rt_x(); x++) {
      for (irt_int y = pin_shape.get_grid_lb_y(); y <= pin_shape.get_grid_rt_y(); y++) {
        PAGCell& pa_gcell = pa_gcell_map[x][y];
        for (PASourceType pa_source_type : {PASourceType::kBlockage, PASourceType::kNetShape}) {
          for (const auto& [curr_net_idx, rect_list] :
               DC_INST.getLayerNetRectMap(pa_gcell.getRegionQuery(pa_source_type), true)[pin_shape_layer_idx]) {
            if (pa_net_idx == curr_net_idx) {
              continue;
            }
            for (const LayerRect& rect : rect_list) {
              for (LayerRect& min_scope_blockage : DC_INST.getMinScope(DRCRect(curr_net_idx, rect, true))) {
                PlanarRect enlarged_rect = RTUtil::getEnlargedRect(min_scope_blockage, half_width);
                if (!RTUtil::isOpenOverlap(pin_shape.get_real_rect(), enlarged_rect)) {
                  continue;
                }
                routing_obs_shape_list.push_back(enlarged_rect);
              }
            }
          }
        }
      }
    }
  }
  std::vector<PlanarRect> wire_legal_rect_list = RTUtil::getClosedCuttingRectListByBoost(origin_pin_shape_list, routing_obs_shape_list);
  std::vector<PlanarRect> reduced_legal_rect_list = RTUtil::getClosedOverlapByBoost(wire_legal_rect_list, wire_reduced_list);
  if (!reduced_legal_rect_list.empty()) {
    wire_legal_rect_list = reduced_legal_rect_list;
  }
  return wire_legal_rect_list;
}

void PinAccessor::mergeAccessPointList(PANet& pa_net)
{
  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    std::map<LayerCoord, AccessPointType, CmpLayerCoordByLayerASC> coord_type_map;
    std::vector<AccessPoint>& access_point_list = pa_pin.get_access_point_list();
    for (AccessPoint& access_point : access_point_list) {
      LayerCoord coord(access_point.get_real_coord(), access_point.get_layer_idx());
      if (RTUtil::exist(coord_type_map, coord)) {
        coord_type_map[coord] = std::min(coord_type_map[coord], access_point.get_type());
      } else {
        coord_type_map[coord] = access_point.get_type();
      }
    }
    access_point_list.clear();
    for (auto& [layer_coord, type] : coord_type_map) {
      access_point_list.emplace_back(layer_coord.get_x(), layer_coord.get_y(), layer_coord.get_layer_idx(), type);
    }
    if (access_point_list.empty()) {
      LOG_INST.error(Loc::current(), "The pin idx ", pa_pin.get_pin_idx(), " access_point_list is empty!");
    }
  }
}

void PinAccessor::updateAccessGridCoord(PANet& pa_net)
{
  updateBoundingBox(pa_net);
  updateAccessGrid(pa_net);
}

void PinAccessor::updateBoundingBox(PANet& pa_net)
{
  ScaleAxis& gcell_axis = DM_INST.getDatabase().get_gcell_axis();

  std::vector<PlanarCoord> coord_list;
  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    for (AccessPoint& access_point : pa_pin.get_access_point_list()) {
      coord_list.push_back(access_point.get_real_coord());
    }
  }
  BoundingBox& bounding_box = pa_net.get_bounding_box();
  bounding_box.set_real_rect(RTUtil::getBoundingBox(coord_list));
  bounding_box.set_grid_rect(RTUtil::getOpenGridRect(bounding_box.get_real_rect(), gcell_axis));
}

void PinAccessor::updateAccessGrid(PANet& pa_net)
{
  ScaleAxis& gcell_axis = DM_INST.getDatabase().get_gcell_axis();
  BoundingBox& bounding_box = pa_net.get_bounding_box();

  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    for (AccessPoint& access_point : pa_pin.get_access_point_list()) {
      access_point.set_grid_coord(RTUtil::getGridCoord(access_point.get_real_coord(), gcell_axis, bounding_box));
    }
  }
}

void PinAccessor::updateViaAccessByDRC(PAModel& pa_model, PANet& pa_net)
{
  irt_int bottom_routing_layer_idx = DM_INST.getConfig().bottom_routing_layer_idx;
  irt_int top_routing_layer_idx = DM_INST.getConfig().top_routing_layer_idx;

  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    for (AccessPoint& access_point : pa_pin.get_access_point_list()) {
      irt_int access_layer_idx = access_point.get_layer_idx();
      std::vector<irt_int> reserved_via_below_layer_idx_list
          = RTUtil::getReservedViaBelowLayerIdxList(access_layer_idx, bottom_routing_layer_idx, top_routing_layer_idx);
      std::sort(reserved_via_below_layer_idx_list.begin(), reserved_via_below_layer_idx_list.end());

      std::vector<DRCRect> drc_rect_list;
      for (irt_int via_below_layer_idx : reserved_via_below_layer_idx_list) {
        std::vector<Segment<LayerCoord>> segment_list;
        segment_list.emplace_back(LayerCoord(access_point.get_real_coord(), via_below_layer_idx),
                                  LayerCoord(access_point.get_real_coord(), via_below_layer_idx + 1));
        for (DRCRect drc_rect : DC_INST.getDRCRectList(pa_net.get_net_idx(), segment_list)) {
          drc_rect_list.push_back(drc_rect);
        }
      }

      if (!hasViolation(pa_model, PASourceType::kBlockage, drc_rect_list)
          && !hasViolation(pa_model, PASourceType::kNetShape, drc_rect_list)) {
        if (access_layer_idx <= reserved_via_below_layer_idx_list.front()) {
          access_point.get_access_orien_set().insert(Orientation::kUp);
        } else {
          access_point.get_access_orien_set().insert(Orientation::kDown);
        }
      }
    }
  }
}

void PinAccessor::updateWireAccessByDRC(PAModel& pa_model, PANet& pa_net)
{
  std::vector<RoutingLayer>& routing_layer_list = DM_INST.getDatabase().get_routing_layer_list();

  GridMap<PAGCell>& pa_gcell_map = pa_model.get_pa_gcell_map();

  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    for (AccessPoint& access_point : pa_pin.get_access_point_list()) {
      LayerCoord curr_grid = access_point.getGridLayerCoord();
      LayerCoord curr_real = access_point.getRealLayerCoord();

      RoutingLayer& routing_layer = routing_layer_list[access_point.get_layer_idx()];
      irt_int half_width = routing_layer.get_min_width() / 2;

      for (Orientation& orientation : routing_layer.getPreferOrientationList()) {
        LayerCoord neighbor_grid = curr_grid;
        if (orientation == Orientation::kEast) {
          neighbor_grid.set_x(curr_grid.get_x() + 1);
        } else if (orientation == Orientation::kWest) {
          neighbor_grid.set_x(curr_grid.get_x() - 1);
        } else if (orientation == Orientation::kSouth) {
          neighbor_grid.set_y(curr_grid.get_y() - 1);
        } else if (orientation == Orientation::kNorth) {
          neighbor_grid.set_y(curr_grid.get_y() + 1);
        }
        if (!pa_gcell_map.isInside(neighbor_grid.get_x(), neighbor_grid.get_y())) {
          continue;
        }
        PlanarRect& base_region = pa_gcell_map[neighbor_grid.get_x()][neighbor_grid.get_y()].get_base_region();
        LayerRect wire;
        if (orientation == Orientation::kEast) {
          wire.set_lb(curr_real.get_x() - half_width, curr_real.get_y() - half_width);
          wire.set_rt(base_region.get_rt_x(), curr_real.get_y() + half_width);
        } else if (orientation == Orientation::kWest) {
          wire.set_lb(base_region.get_lb_x(), curr_real.get_y() - half_width);
          wire.set_rt(curr_real.get_x() + half_width, curr_real.get_y() + half_width);
        } else if (orientation == Orientation::kSouth) {
          wire.set_lb(curr_real.get_x() - half_width, base_region.get_lb_y());
          wire.set_rt(curr_real.get_x() + half_width, curr_real.get_y() + half_width);
        } else if (orientation == Orientation::kNorth) {
          wire.set_lb(curr_real.get_x() - half_width, curr_real.get_y() - half_width);
          wire.set_rt(curr_real.get_x() + half_width, base_region.get_rt_y());
        }
        wire.set_layer_idx(curr_real.get_layer_idx());
        DRCRect drc_rect(pa_net.get_net_idx(), wire, true);
        if (!hasViolation(pa_model, PASourceType::kBlockage, drc_rect) && !hasViolation(pa_model, PASourceType::kNetShape, drc_rect)) {
          access_point.get_access_orien_set().insert(orientation);
        }
      }
    }
  }
}

void PinAccessor::updateWeakAccessByDRC(PAModel& pa_model, PANet& pa_net)
{
  std::vector<RoutingLayer>& routing_layer_list = DM_INST.getDatabase().get_routing_layer_list();

  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    bool has_access = false;
    for (AccessPoint& access_point : pa_pin.get_access_point_list()) {
      if (!access_point.get_access_orien_set().empty()) {
        has_access = true;
        break;
      }
    }
    if (has_access) {
      continue;
    }
    /**
     * 对于没有任何access的pa_pin做最后的access补充
     *
     * weak线段满足以下条件
     *    在pin_shape之外的长度为
     *    max
     *    (pin_shape的形状的bounding_box的min_spacing + min_width,
     *    当前层的non-prefer的pitch)
     */
    std::map<irt_int, std::vector<PlanarRect>> layer_pin_shape_map;
    for (EXTLayerRect& routing_shape : pa_pin.get_routing_shape_list()) {
      layer_pin_shape_map[routing_shape.get_layer_idx()].push_back(routing_shape.get_real_rect());
    }
    std::map<irt_int, PlanarRect> layer_base_region_map;
    for (auto& [layer_idx, pin_shape_list] : layer_pin_shape_map) {
      layer_base_region_map[layer_idx] = RTUtil::getBoundingBox(pin_shape_list);
    }
    for (auto& [layer_idx, base_region] : layer_base_region_map) {
      RoutingLayer& routing_layer = routing_layer_list[layer_idx];
      irt_int spacing_length = routing_layer.get_min_width() + routing_layer.getMinSpacing(base_region);
      irt_int pitch_length = routing_layer.getNonpreferTrackGridList().front().get_step_length();
      base_region = RTUtil::getEnlargedRect(base_region, std::max(spacing_length, pitch_length));
    }
    for (AccessPoint& access_point : pa_pin.get_access_point_list()) {
      LayerCoord curr_real = access_point.getRealLayerCoord();

      RoutingLayer& routing_layer = routing_layer_list[access_point.get_layer_idx()];
      irt_int half_width = routing_layer.get_min_width() / 2;

      PlanarRect& base_region = layer_base_region_map[access_point.get_layer_idx()];
      for (Orientation& orientation : routing_layer.getPreferOrientationList()) {
        LayerRect wire;
        if (orientation == Orientation::kEast) {
          wire.set_lb(curr_real.get_x() - half_width, curr_real.get_y() - half_width);
          wire.set_rt(base_region.get_rt_x(), curr_real.get_y() + half_width);
        } else if (orientation == Orientation::kWest) {
          wire.set_lb(base_region.get_lb_x(), curr_real.get_y() - half_width);
          wire.set_rt(curr_real.get_x() + half_width, curr_real.get_y() + half_width);
        } else if (orientation == Orientation::kSouth) {
          wire.set_lb(curr_real.get_x() - half_width, base_region.get_lb_y());
          wire.set_rt(curr_real.get_x() + half_width, curr_real.get_y() + half_width);
        } else if (orientation == Orientation::kNorth) {
          wire.set_lb(curr_real.get_x() - half_width, curr_real.get_y() - half_width);
          wire.set_rt(curr_real.get_x() + half_width, base_region.get_rt_y());
        }
        wire.set_layer_idx(curr_real.get_layer_idx());
        DRCRect drc_rect(pa_net.get_net_idx(), wire, true);
        if (!hasViolation(pa_model, PASourceType::kBlockage, drc_rect) && !hasViolation(pa_model, PASourceType::kNetShape, drc_rect)) {
          access_point.get_access_orien_set().insert(orientation);
        }
      }
    }
  }
}

void PinAccessor::initProtectedPointList(PANet& pa_net)
{
  irt_int bottom_routing_layer_idx = DM_INST.getConfig().bottom_routing_layer_idx;
  irt_int top_routing_layer_idx = DM_INST.getConfig().top_routing_layer_idx;

  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    std::vector<AccessPoint>& protected_point_list = pa_pin.get_protected_point_list();
    for (AccessPoint& access_point : pa_pin.get_access_point_list()) {
      if (!access_point.get_access_orien_set().empty()) {
        protected_point_list.push_back(access_point);
      }
    }
    if (protected_point_list.empty()) {
      LOG_INST.warning(Loc::current(), "The protected_point_list is empty!");
      for (AccessPoint& access_point : pa_pin.get_access_point_list()) {
        if (access_point.get_layer_idx() <= bottom_routing_layer_idx) {
          access_point.get_access_orien_set().insert(Orientation::kUp);
        } else if (top_routing_layer_idx <= access_point.get_layer_idx()) {
          access_point.get_access_orien_set().insert(Orientation::kDown);
        } else {
          access_point.get_access_orien_set().insert(Orientation::kUp);
        }
        protected_point_list.push_back(access_point);
      }
    }
  }
}

void PinAccessor::checkProtectedPointList(PANet& pa_net)
{
  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    std::vector<AccessPoint>& protected_point_list = pa_pin.get_protected_point_list();
    if (protected_point_list.empty()) {
      LOG_INST.error(Loc::current(), "The pin ", pa_pin.get_pin_idx(), " access point list is empty!");
    }

    for (AccessPoint& access_point : protected_point_list) {
      if (access_point.get_type() == AccessPointType::kNone) {
        LOG_INST.error(Loc::current(), "The access point type is wrong!");
      }
      bool is_legal = false;
      for (EXTLayerRect& routing_shape : pa_pin.get_routing_shape_list()) {
        if (routing_shape.get_layer_idx() == access_point.get_layer_idx()
            && RTUtil::isInside(routing_shape.get_real_rect(), access_point.get_real_coord())) {
          is_legal = true;
          break;
        }
      }
      if (!is_legal) {
        LOG_INST.error(Loc::current(), "The access point is not in routing shape!");
      }
    }
  }
}

void PinAccessor::updateNetCandidateViaMap(PAModel& pa_model)
{
  irt_int bottom_routing_layer_idx = DM_INST.getConfig().bottom_routing_layer_idx;
  irt_int top_routing_layer_idx = DM_INST.getConfig().top_routing_layer_idx;

  for (PANet& pa_net : pa_model.get_pa_net_list()) {
    for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
      for (AccessPoint& access_point : pa_pin.get_protected_point_list()) {
        std::set<Orientation>& access_orien_set = access_point.get_access_orien_set();
        if (!RTUtil::exist(access_orien_set, Orientation::kUp) && !RTUtil::exist(access_orien_set, Orientation::kDown)) {
          continue;
        }
        for (irt_int via_below_layer_idx :
             RTUtil::getReservedViaBelowLayerIdxList(access_point.get_layer_idx(), bottom_routing_layer_idx, top_routing_layer_idx)) {
          std::vector<Segment<LayerCoord>> segment_list;
          segment_list.emplace_back(LayerCoord(access_point.get_real_coord(), via_below_layer_idx),
                                    LayerCoord(access_point.get_real_coord(), via_below_layer_idx + 1));
          for (DRCRect& drc_rect : DC_INST.getDRCRectList(pa_net.get_net_idx(), segment_list)) {
            addRectToEnv(pa_model, PASourceType::kCandidateVia, drc_rect);
          }
        }
      }
    }
  }
}

void PinAccessor::filterPANetList(PAModel& pa_model)
{
  Monitor monitor;

  std::vector<PANet>& pa_net_list = pa_model.get_pa_net_list();

  irt_int batch_size = RTUtil::getBatchSize(pa_net_list.size());

  Monitor stage_monitor;

#pragma omp parallel for
  for (size_t i = 0; i < pa_net_list.size(); i++) {
    PANet& pa_net = pa_net_list[i];
    filterGoodByViaConflict(pa_net, pa_model);
    filterGoodByAccessOrienSet(pa_net);
    filterGoodByAccessPointType(pa_net);
    filterGoodByNetDistance(pa_net);
    initProtectedAccessPoint(pa_net);
    if (omp_get_num_threads() == 1 && (i + 1) % batch_size == 0) {
      LOG_INST.info(Loc::current(), "Filter ", (i + 1), " nets", stage_monitor.getStatsInfo());
    }
  }
  if (omp_get_num_threads() == 1) {
    LOG_INST.info(Loc::current(), "Filter ", pa_net_list.size(), " nets", monitor.getStatsInfo());
  }
}

void PinAccessor::filterGoodByViaConflict(PANet& pa_net, PAModel& pa_model)
{
  irt_int bottom_routing_layer_idx = DM_INST.getConfig().bottom_routing_layer_idx;
  irt_int top_routing_layer_idx = DM_INST.getConfig().top_routing_layer_idx;

  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    std::vector<AccessPoint>& protected_point_list = pa_pin.get_protected_point_list();
    std::map<irt_int, std::vector<AccessPoint>, std::greater<irt_int>> via_num_access_point_map;
    for (AccessPoint& access_point : protected_point_list) {
      irt_int access_layer_idx = access_point.get_layer_idx();
      std::set<Orientation>& access_orien_set = access_point.get_access_orien_set();
      if (!RTUtil::exist(access_orien_set, Orientation::kUp) && !RTUtil::exist(access_orien_set, Orientation::kDown)) {
        via_num_access_point_map[0].push_back(access_point);
        continue;
      }
      irt_int via_num = 0;
      for (irt_int via_below_layer_idx :
           RTUtil::getReservedViaBelowLayerIdxList(access_layer_idx, bottom_routing_layer_idx, top_routing_layer_idx)) {
        std::vector<Segment<LayerCoord>> segment_list;
        segment_list.emplace_back(LayerCoord(access_point.get_real_coord(), via_below_layer_idx),
                                  LayerCoord(access_point.get_real_coord(), via_below_layer_idx + 1));
        std::vector<DRCRect> drc_rect_list = DC_INST.getDRCRectList(pa_net.get_net_idx(), segment_list);
        if (!hasViolation(pa_model, PASourceType::kCandidateVia, drc_rect_list)) {
          via_num++;
        } else {
          break;
        }
      }
      via_num_access_point_map[via_num].push_back(access_point);
    }
    protected_point_list.clear();
    if (!via_num_access_point_map.empty()) {
      for (AccessPoint& access_point : via_num_access_point_map.begin()->second) {
        protected_point_list.push_back(access_point);
      }
    }
  }
}

void PinAccessor::filterGoodByAccessOrienSet(PANet& pa_net)
{
  irt_int bottom_routing_layer_idx = DM_INST.getConfig().bottom_routing_layer_idx;
  irt_int top_routing_layer_idx = DM_INST.getConfig().top_routing_layer_idx;

  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    std::map<irt_int, std::vector<AccessPoint>, std::greater<irt_int>> access_num_point_map;
    for (AccessPoint& access_point : pa_pin.get_protected_point_list()) {
      std::set<Orientation>& access_orien_set = access_point.get_access_orien_set();
      if (access_point.get_layer_idx() < bottom_routing_layer_idx) {
        for (Orientation orientation :
             {Orientation::kEast, Orientation::kWest, Orientation::kSouth, Orientation::kNorth, Orientation::kDown}) {
          access_orien_set.erase(orientation);
        }
      }
      if (top_routing_layer_idx < access_point.get_layer_idx()) {
        for (Orientation orientation :
             {Orientation::kEast, Orientation::kWest, Orientation::kSouth, Orientation::kNorth, Orientation::kUp}) {
          access_orien_set.erase(orientation);
        }
      }
      access_num_point_map[access_orien_set.size()].push_back(access_point);
    }
    if (!access_num_point_map.empty()) {
      pa_pin.set_protected_point_list(access_num_point_map.begin()->second);
    }
  }
}

void PinAccessor::filterGoodByAccessPointType(PANet& pa_net)
{
  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    std::vector<AccessPoint>& pin_protected_point_list = pa_pin.get_protected_point_list();
    std::map<irt_int, std::map<AccessPointType, std::vector<AccessPoint>>> layer_access_point_map;
    for (AccessPoint& access_point : pin_protected_point_list) {
      layer_access_point_map[access_point.get_layer_idx()][access_point.get_type()].push_back(access_point);
    }
    pin_protected_point_list.clear();
    for (auto& [layer_idx, type_point_map] : layer_access_point_map) {
      for (AccessPointType access_point_type :
           {AccessPointType::kPrefTrackGrid, AccessPointType::kCurrTrackGrid, AccessPointType::kOnTrack, AccessPointType::kOnShape}) {
        std::vector<AccessPoint>& candidate_protected_point_list = type_point_map[access_point_type];
        if (candidate_protected_point_list.empty()) {
          continue;
        }
        for (AccessPoint& access_point : candidate_protected_point_list) {
          pin_protected_point_list.push_back(access_point);
        }
        break;
      }
    }
  }
}

void PinAccessor::filterGoodByNetDistance(PANet& pa_net)
{
  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    std::vector<LayerCoord> real_coord_list;
    for (AccessPoint& access_point : pa_pin.get_protected_point_list()) {
      real_coord_list.push_back(access_point.getRealLayerCoord());
    }
    LayerCoord balance_coord = RTUtil::getBalanceCoord(real_coord_list);
    std::map<irt_int, std::vector<AccessPoint>> distance_access_point_map;
    std::vector<AccessPoint>& pin_protected_point_list = pa_pin.get_protected_point_list();
    for (AccessPoint& access_point : pa_pin.get_protected_point_list()) {
      irt_int distance = RTUtil::getManhattanDistance(balance_coord, access_point.getRealLayerCoord());
      distance_access_point_map[distance].push_back(access_point);
    }
    if (!distance_access_point_map.empty()) {
      pin_protected_point_list.clear();
      pin_protected_point_list.push_back(distance_access_point_map.begin()->second.front());
    }
  }
}

void PinAccessor::initProtectedAccessPoint(PANet& pa_net)
{
  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    if (pa_pin.get_protected_point_list().size() != 1) {
      LOG_INST.warning(Loc::current(), "The number of protected_point_list is not 1, which may affect optimization!");
    }
    pa_pin.set_protected_access_point(pa_pin.get_protected_point_list().front());
  }
}

void PinAccessor::processPAModel(PAModel& pa_model)
{
  updateAccessPointList(pa_model);
  updateNetAccessPointMap(pa_model);
}

void PinAccessor::updateAccessPointList(PAModel& pa_model)
{
#pragma omp parallel for
  for (PANet& pa_net : pa_model.get_pa_net_list()) {
    updateDrivingPin(pa_net);
  }
}

void PinAccessor::updateDrivingPin(PANet& pa_net)
{
  PAPin& pa_driving_pin = pa_net.get_pa_driving_pin();
  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    if (pa_driving_pin.get_pin_idx() != pa_pin.get_pin_idx()) {
      continue;
    }
    pa_driving_pin = pa_pin;
    return;
  }
  LOG_INST.error(Loc::current(), "Unable to find a driving pin!");
}

void PinAccessor::updateNetAccessPointMap(PAModel& pa_model)
{
  irt_int bottom_routing_layer_idx = DM_INST.getConfig().bottom_routing_layer_idx;
  irt_int top_routing_layer_idx = DM_INST.getConfig().top_routing_layer_idx;

  for (PANet& pa_net : pa_model.get_pa_net_list()) {
    for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
      AccessPoint& protected_access_point = pa_pin.get_protected_access_point();
      std::set<Orientation>& access_orien_set = protected_access_point.get_access_orien_set();
      if (!RTUtil::exist(access_orien_set, Orientation::kUp) && !RTUtil::exist(access_orien_set, Orientation::kDown)) {
        continue;
      }
      for (irt_int via_below_layer_idx : RTUtil::getReservedViaBelowLayerIdxList(protected_access_point.get_layer_idx(),
                                                                                 bottom_routing_layer_idx, top_routing_layer_idx)) {
        std::vector<Segment<LayerCoord>> segment_list;
        segment_list.emplace_back(LayerCoord(protected_access_point.get_real_coord(), via_below_layer_idx),
                                  LayerCoord(protected_access_point.get_real_coord(), via_below_layer_idx + 1));
        for (DRCRect& drc_rect : DC_INST.getDRCRectList(pa_net.get_net_idx(), segment_list)) {
          addRectToEnv(pa_model, PASourceType::kNetShape, drc_rect);
        }
      }
    }
  }
}

void PinAccessor::countPAModel(PAModel& pa_model)
{
  PAModelStat pa_model_stat;

  std::map<AccessPointType, irt_int>& type_pin_num_map = pa_model_stat.get_type_pin_num_map();
  std::map<irt_int, irt_int>& routing_port_num_map = pa_model_stat.get_routing_port_num_map();
  std::map<irt_int, irt_int>& routing_access_point_num_map = pa_model_stat.get_routing_access_point_num_map();

  for (PANet& pa_net : pa_model.get_pa_net_list()) {
    for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
      AccessPointType access_point_type = pa_pin.get_protected_access_point().get_type();
      type_pin_num_map[access_point_type]++;
    }
    for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
      std::set<irt_int> routing_layer_idx_set;
      for (EXTLayerRect& routing_shape : pa_pin.get_routing_shape_list()) {
        routing_layer_idx_set.insert(routing_shape.get_layer_idx());
      }
      for (irt_int routing_layer_idx : routing_layer_idx_set) {
        routing_port_num_map[routing_layer_idx]++;
      }
    }
    for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
      routing_access_point_num_map[pa_pin.get_protected_access_point().get_layer_idx()]++;
    }
  }

  std::map<PASourceType, std::map<irt_int, std::map<std::string, std::vector<ViolationInfo>>>>& source_routing_drc_violation_map
      = pa_model_stat.get_source_routing_drc_violation_map();
  std::map<PASourceType, std::map<irt_int, std::map<std::string, std::vector<ViolationInfo>>>>& source_cut_drc_violation_map
      = pa_model_stat.get_source_cut_drc_violation_map();
  GridMap<PAGCell>& pa_gcell_map = pa_model.get_pa_gcell_map();
  for (irt_int x = 0; x < pa_gcell_map.get_x_size(); x++) {
    for (irt_int y = 0; y < pa_gcell_map.get_y_size(); y++) {
      PAGCell& pa_gcell = pa_gcell_map[x][y];

      std::vector<DRCRect> drc_rect_list;
      for (bool is_routing : {true, false}) {
        for (auto& [layer_idx, net_rect_map] : DC_INST.getLayerNetRectMap(pa_gcell.getRegionQuery(PASourceType::kNetShape), is_routing)) {
          for (auto& [net_idx, rect_set] : net_rect_map) {
            for (const LayerRect& rect : rect_set) {
              drc_rect_list.emplace_back(net_idx, rect, is_routing);
            }
          }
        }
      }

      for (PASourceType pa_source_type : {PASourceType::kBlockage, PASourceType::kNetShape}) {
        for (auto& [drc, violation_info_list] : getViolationInfo(pa_gcell, pa_source_type, drc_rect_list)) {
          for (ViolationInfo& violation_info : violation_info_list) {
            irt_int layer_idx = violation_info.get_violation_region().get_layer_idx();
            if (violation_info.get_is_routing()) {
              source_routing_drc_violation_map[pa_source_type][layer_idx][drc].push_back(violation_info);
            } else {
              source_cut_drc_violation_map[pa_source_type][layer_idx][drc].push_back(violation_info);
            }
          }
        }
      }
    }
  }

  irt_int total_pin_num = 0;
  irt_int total_port_num = 0;
  irt_int total_access_point_num = 0;
  irt_int total_drc_number = 0;
  for (auto& [type, pin_num] : type_pin_num_map) {
    total_pin_num += pin_num;
  }
  for (auto& [routing_layer_idx, port_num] : routing_port_num_map) {
    total_port_num += port_num;
  }
  for (auto& [routing_layer_idx, access_point_num] : routing_access_point_num_map) {
    total_access_point_num += access_point_num;
  }
  for (auto& [pa_source_type, routing_drc_violation_map] : source_routing_drc_violation_map) {
    for (auto& [layer_idx, drc_violation_list_map] : routing_drc_violation_map) {
      for (auto& [drc, violation_list] : drc_violation_list_map) {
        total_drc_number += violation_list.size();
      }
    }
  }
  for (auto& [pa_source_type, cut_drc_violation_map] : source_cut_drc_violation_map) {
    for (auto& [layer_idx, drc_violation_list_map] : cut_drc_violation_map) {
      for (auto& [drc, violation_list] : drc_violation_list_map) {
        total_drc_number += violation_list.size();
      }
    }
  }
  pa_model_stat.set_total_pin_num(total_pin_num);
  pa_model_stat.set_total_port_num(total_port_num);
  pa_model_stat.set_total_access_point_num(total_access_point_num);
  pa_model_stat.set_total_drc_number(total_drc_number);

  pa_model.set_pa_model_stat(pa_model_stat);
}

void PinAccessor::reportPAModel(PAModel& pa_model)
{
  std::vector<RoutingLayer>& routing_layer_list = DM_INST.getDatabase().get_routing_layer_list();
  std::vector<CutLayer>& cut_layer_list = DM_INST.getDatabase().get_cut_layer_list();

  PAModelStat& pa_model_stat = pa_model.get_pa_model_stat();
  std::map<AccessPointType, irt_int>& type_pin_num_map = pa_model_stat.get_type_pin_num_map();
  irt_int total_pin_num = pa_model_stat.get_total_pin_num();
  std::map<irt_int, irt_int>& routing_port_num_map = pa_model_stat.get_routing_port_num_map();
  irt_int total_port_num = pa_model_stat.get_total_port_num();
  std::map<irt_int, irt_int>& routing_access_point_num_map = pa_model_stat.get_routing_access_point_num_map();
  irt_int total_access_point_num = pa_model_stat.get_total_access_point_num();

  // report pin info
  fort::char_table pin_table;
  pin_table << fort::header << "Access Type"
            << "Pin Number" << fort::endr;
  for (AccessPointType access_point_type :
       {AccessPointType::kPrefTrackGrid, AccessPointType::kCurrTrackGrid, AccessPointType::kOnTrack, AccessPointType::kOnShape}) {
    pin_table << GetAccessPointTypeName()(access_point_type)
              << RTUtil::getString(type_pin_num_map[access_point_type], "(",
                                   RTUtil::getPercentage(type_pin_num_map[access_point_type], total_pin_num), "%)")
              << fort::endr;
  }
  pin_table << fort::header << "Total" << total_pin_num << fort::endr;

  // report port info
  fort::char_table port_table;
  port_table << fort::header << "Routing Layer"
             << "Port Number"
             << "Access Point Number" << fort::endr;
  for (RoutingLayer& routing_layer : routing_layer_list) {
    irt_int routing_layer_idx = routing_layer.get_layer_idx();
    port_table << routing_layer.get_layer_name()
               << RTUtil::getString(routing_port_num_map[routing_layer_idx], "(",
                                    RTUtil::getPercentage(routing_port_num_map[routing_layer_idx], total_port_num), "%)")
               << RTUtil::getString(routing_access_point_num_map[routing_layer_idx], "(",
                                    RTUtil::getPercentage(routing_access_point_num_map[routing_layer_idx], total_access_point_num), "%)")
               << fort::endr;
  }
  port_table << fort::header << "Total" << total_port_num << total_access_point_num << fort::endr;

  // print
  RTUtil::printTableList({pin_table, port_table});

  // build drc table
  std::vector<fort::char_table> source_drc_table_list;
  for (auto& [source, routing_drc_violation_map] : pa_model_stat.get_source_routing_drc_violation_map()) {
    source_drc_table_list.push_back(RTUtil::buildDRCTable(routing_layer_list, GetPASourceTypeName()(source), routing_drc_violation_map));
  }
  for (auto& [source, cut_drc_violation_map] : pa_model_stat.get_source_cut_drc_violation_map()) {
    source_drc_table_list.push_back(RTUtil::buildDRCTable(cut_layer_list, GetPASourceTypeName()(source), cut_drc_violation_map));
  }
  // print
  RTUtil::printTableList(source_drc_table_list);
}

bool PinAccessor::stopPAModel(PAModel& pa_model)
{
  return false;
}

#endif

#if 1  // update

void PinAccessor::update(PAModel& pa_model)
{
  for (PANet& pa_net : pa_model.get_pa_net_list()) {
    std::vector<Pin>& pin_list = pa_net.get_origin_net()->get_pin_list();
    for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
      Pin& pin = pin_list[pa_pin.get_pin_idx()];
      if (pin.get_pin_idx() != pa_pin.get_pin_idx()) {
        LOG_INST.error(Loc::current(), "The pin idx is not equal!");
      }
      pin.set_protected_access_point(pa_pin.get_protected_access_point());
      pin.set_access_point_list(pa_pin.get_access_point_list());
    }

    Pin& driving_pin = pa_net.get_origin_net()->get_driving_pin();
    PAPin& pa_driving_pin = pa_net.get_pa_driving_pin();
    if (driving_pin.get_pin_idx() != pa_driving_pin.get_pin_idx()) {
      LOG_INST.error(Loc::current(), "The pin idx is not equal!");
    }
    driving_pin.set_protected_access_point(pa_driving_pin.get_protected_access_point());
    driving_pin.set_access_point_list(pa_driving_pin.get_access_point_list());

    pa_net.get_origin_net()->set_bounding_box(pa_net.get_bounding_box());
  }
}

#endif

#if 1  // valid drc

bool PinAccessor::hasViolation(PAModel& pa_model, PASourceType pa_source_type, const DRCRect& drc_rect)
{
  std::vector<DRCRect> drc_rect_list = {drc_rect};
  return hasViolation(pa_model, pa_source_type, drc_rect_list);
}

bool PinAccessor::hasViolation(PAModel& pa_model, PASourceType pa_source_type, const std::vector<DRCRect>& drc_rect_list)
{
  ScaleAxis& gcell_axis = DM_INST.getDatabase().get_gcell_axis();
  EXTPlanarRect& die = DM_INST.getDatabase().get_die();

  GridMap<PAGCell>& pa_gcell_map = pa_model.get_pa_gcell_map();

  std::map<PAGCellId, std::vector<DRCRect>, CmpPAGCellId> gcell_rect_map;
  for (const DRCRect& drc_rect : drc_rect_list) {
    for (const LayerRect& max_scope_real_rect : DC_INST.getMaxScope(drc_rect)) {
      PlanarRect max_scope_regular_rect = RTUtil::getRegularRect(max_scope_real_rect, die.get_real_rect());
      PlanarRect max_scope_grid_rect = RTUtil::getClosedGridRect(max_scope_regular_rect, gcell_axis);
      for (irt_int x = max_scope_grid_rect.get_lb_x(); x <= max_scope_grid_rect.get_rt_x(); x++) {
        for (irt_int y = max_scope_grid_rect.get_lb_y(); y <= max_scope_grid_rect.get_rt_y(); y++) {
          gcell_rect_map[PAGCellId(x, y)].push_back(drc_rect);
        }
      }
    }
  }
  bool has_violation = false;
  for (const auto& [pa_gcell_id, drc_rect_list] : gcell_rect_map) {
    PAGCell& pa_gcell = pa_gcell_map[pa_gcell_id.get_x()][pa_gcell_id.get_y()];
    if (getViolationInfo(pa_gcell, pa_source_type, drc_rect_list).size() > 0) {
      has_violation = true;
      break;
    }
  }
  return has_violation;
}

std::map<std::string, std::vector<ViolationInfo>> PinAccessor::getViolationInfo(PAGCell& pa_gcell, PASourceType pa_source_type,
                                                                                const std::vector<DRCRect>& drc_rect_list)
{
  std::map<std::string, std::vector<ViolationInfo>> drc_violation_map;
  drc_violation_map = DC_INST.getViolationInfo(pa_gcell.getRegionQuery(pa_source_type), drc_rect_list);
  removeInvalidViolationInfo(pa_gcell, drc_violation_map);
  return drc_violation_map;
}

std::map<std::string, std::vector<ViolationInfo>> PinAccessor::getViolationInfo(PAGCell& pa_gcell, PASourceType pa_source_type)
{
  std::map<std::string, std::vector<ViolationInfo>> drc_violation_map;
  drc_violation_map = DC_INST.getViolationInfo(pa_gcell.getRegionQuery(pa_source_type));
  removeInvalidViolationInfo(pa_gcell, drc_violation_map);
  return drc_violation_map;
}

void PinAccessor::removeInvalidViolationInfo(PAGCell& pa_gcell, std::map<std::string, std::vector<ViolationInfo>>& drc_violation_map)
{
  for (auto& [drc, violation_list] : drc_violation_map) {
    std::vector<ViolationInfo> valid_violation_list;
    for (ViolationInfo& violation_info : violation_list) {
      bool is_valid = false;
      for (auto& [net_idx, rect_list] : violation_info.get_net_shape_map()) {
        if (net_idx != -1) {
          is_valid = true;
          break;
        }
      }
      if (is_valid) {
        valid_violation_list.push_back(violation_info);
      }
    }
    if (valid_violation_list.empty()) {
      drc_violation_map.erase(drc);
    } else {
      drc_violation_map[drc] = violation_list;
    }
  }
}

#endif

}  // namespace irt
