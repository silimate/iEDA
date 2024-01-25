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
  accessNetList(net_list);
}

// private

PinAccessor* PinAccessor::_pa_instance = nullptr;

void PinAccessor::accessNetList(std::vector<Net>& net_list)
{
  Monitor monitor;
  LOG_INST.info(Loc::current(), "Begin accessing...");

  PAModel pa_model = initPAModel(net_list);
  accessPANetList(pa_model);
  plotPAModel(pa_model, "post");

  LOG_INST.info(Loc::current(), "End access", monitor.getStatsInfo());
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
      pa_gcell.set_base_region(RTUtil::getRealRectByGCell(x, y, gcell_axis));
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

void PinAccessor::accessPANetList(PAModel& pa_model)
{
#pragma omp parallel for
  for (PANet& pa_net : pa_model.get_pa_net_list()) {
    makeAccessPointList(pa_model, pa_net);
    // mergeAccessPointList(pa_net);
    updateBoundingBox(pa_net);
    updateAccessGrid(pa_net);
  }
}

void PinAccessor::makeAccessPointList(PAModel& pa_model, PANet& pa_net)
{
  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    std::vector<AccessPoint>& access_point_list = pa_pin.get_access_point_list();
    std::vector<LayerRect> legal_shape_list = getLegalShapeList(pa_model, pa_net.get_net_idx(), pa_pin);
    for (auto getAccessPointList : {std::bind(&PinAccessor::getAccessPointListByPrefTrackGrid, this, std::placeholders::_1),
                                    std::bind(&PinAccessor::getAccessPointListByCurrTrackGrid, this, std::placeholders::_1),
                                    std::bind(&PinAccessor::getAccessPointListByOnTrack, this, std::placeholders::_1),
                                    std::bind(&PinAccessor::getAccessPointListByOnShape, this, std::placeholders::_1)}) {
      for (AccessPoint& access_point : getAccessPointList(legal_shape_list)) {
        access_point_list.push_back(access_point);
      }
      if (!access_point_list.empty()) {
        break;
      }
    }
    if (access_point_list.empty()) {
      LOG_INST.error(Loc::current(), "No access point was generated!");
    }
  }
}

std::vector<LayerRect> PinAccessor::getLegalShapeList(PAModel& pa_model, irt_int pa_net_idx, PAPin& pa_pin)
{
  std::map<irt_int, std::vector<EXTLayerRect>> layer_pin_shape_list;
  for (EXTLayerRect& routing_shape : pa_pin.get_routing_shape_list()) {
    layer_pin_shape_list[routing_shape.get_layer_idx()].emplace_back(routing_shape);
  }
  std::vector<LayerRect> legal_rect_list;
  for (auto& [layer_idx, pin_shpae_list] : layer_pin_shape_list) {
    for (PlanarRect planar_legal_rect : getPlanarLegalRectList(pa_model, pa_net_idx, pin_shpae_list)) {
      legal_rect_list.emplace_back(planar_legal_rect, layer_idx);
    }
  }
  if (!legal_rect_list.empty()) {
    return legal_rect_list;
  }
  LOG_INST.warn(Loc::current(), "The pin ", pa_pin.get_pin_name(), " without legal shape!");
  for (EXTLayerRect& routing_shape : pa_pin.get_routing_shape_list()) {
    legal_rect_list.emplace_back(routing_shape.getRealLayerRect());
  }
  return legal_rect_list;
}

std::vector<PlanarRect> PinAccessor::getPlanarLegalRectList(PAModel& pa_model, irt_int pa_net_idx,
                                                            std::vector<EXTLayerRect>& pin_shape_list)
{
  std::vector<RoutingLayer>& routing_layer_list = DM_INST.getDatabase().get_routing_layer_list();

  for (EXTLayerRect& pin_shape : pin_shape_list) {
    if (pin_shape_list.front().get_layer_idx() != pin_shape.get_layer_idx()) {
      LOG_INST.error(Loc::current(), "The pin_shape_list is not on the same layer!");
    }
  }
  irt_int pin_shape_layer_idx = pin_shape_list.front().get_layer_idx();

  std::vector<PlanarRect> origin_pin_shape_list;
  for (EXTLayerRect& pin_shape : pin_shape_list) {
    origin_pin_shape_list.push_back(pin_shape.get_real_rect());
  }
  // 当前层缩小后的结果
  irt_int reduced_size = routing_layer_list[pin_shape_layer_idx].get_min_width() / 2;
  std::vector<PlanarRect> reduced_rect_list = RTUtil::getClosedReducedRectListByBoost(origin_pin_shape_list, reduced_size);

  // 要被剪裁的blockage的集合 排序按照 本层 上层
  /**
   * 要被剪裁的blockage的集合
   * 如果不是最顶层就往上取一层
   * 是最顶层就往下取一层
   */
  std::vector<irt_int> pin_layer_idx_list;
  if (pin_shape_layer_idx < (static_cast<irt_int>(routing_layer_list.size()) - 1)) {
    pin_layer_idx_list = {pin_shape_layer_idx, pin_shape_layer_idx + 1};
  } else {
    pin_layer_idx_list = {pin_shape_layer_idx, pin_shape_layer_idx - 1};
  }
  std::vector<std::vector<PlanarRect>> routing_obs_shape_list_list;
  for (irt_int layer_idx : pin_layer_idx_list) {
    RoutingLayer& routing_layer = routing_layer_list[layer_idx];
    std::vector<PlanarRect> routing_obs_shape_list;
    for (EXTLayerRect& pin_shape : pin_shape_list) {
      auto net_fixed_rect_map = DM_INST.getTypeLayerNetFixedRectMap(pin_shape)[true][layer_idx];
      for (auto& [net_idx, rect_set] : net_fixed_rect_map) {
        if (net_idx == pa_net_idx) {
          continue;
        }
        for (EXTLayerRect* rect : rect_set) {
          irt_int enlarged_size = routing_layer.getMinSpacing(rect->get_real_rect()) + (routing_layer.get_min_width() / 2);
          PlanarRect enlarged_rect = RTUtil::getEnlargedRect(rect->get_real_rect(), enlarged_size);
          if (RTUtil::isOpenOverlap(pin_shape.get_real_rect(), enlarged_rect)) {
            routing_obs_shape_list.push_back(enlarged_rect);
          }
        }
      }
    }
    if (!routing_obs_shape_list.empty()) {
      routing_obs_shape_list_list.push_back(routing_obs_shape_list);
    }
  }
  std::vector<PlanarRect> legal_rect_list = reduced_rect_list;
  for (std::vector<PlanarRect>& routing_obs_shape_list : routing_obs_shape_list_list) {
    std::vector<PlanarRect> legal_rect_list_temp = RTUtil::getClosedCuttingRectListByBoost(legal_rect_list, routing_obs_shape_list);
    if (!legal_rect_list_temp.empty()) {
      legal_rect_list = legal_rect_list_temp;
    } else {
      break;
    }
  }
  return legal_rect_list;
}

std::vector<AccessPoint> PinAccessor::getAccessPointListByPrefTrackGrid(std::vector<LayerRect>& legal_shape_list)
{
  std::vector<RoutingLayer>& routing_layer_list = DM_INST.getDatabase().get_routing_layer_list();

  std::vector<LayerCoord> layer_coord_list;
  for (LayerRect& legal_shape : legal_shape_list) {
    irt_int lb_x = legal_shape.get_lb_x();
    irt_int lb_y = legal_shape.get_lb_y();
    irt_int rt_x = legal_shape.get_rt_x();
    irt_int rt_y = legal_shape.get_rt_y();
    irt_int curr_layer_idx = legal_shape.get_layer_idx();
    irt_int other_layer_idx;
    if (curr_layer_idx < (static_cast<irt_int>(routing_layer_list.size()) - 1)) {
      other_layer_idx = curr_layer_idx + 1;
    } else {
      other_layer_idx = curr_layer_idx - 1;
    }
    // prefer track grid
    RoutingLayer curr_routing_layer = routing_layer_list[curr_layer_idx];
    RoutingLayer other_routing_layer = routing_layer_list[other_layer_idx];
    if (curr_routing_layer.isPreferH()) {
      for (irt_int x : RTUtil::getClosedScaleList(lb_x, rt_x, other_routing_layer.getPreferTrackGridList())) {
        for (irt_int y : RTUtil::getClosedScaleList(lb_y, rt_y, curr_routing_layer.getPreferTrackGridList())) {
          layer_coord_list.emplace_back(x, y, curr_layer_idx);
        }
      }
    } else {
      for (irt_int x : RTUtil::getClosedScaleList(lb_x, rt_x, curr_routing_layer.getPreferTrackGridList())) {
        for (irt_int y : RTUtil::getClosedScaleList(lb_y, rt_y, other_routing_layer.getPreferTrackGridList())) {
          layer_coord_list.emplace_back(x, y, curr_layer_idx);
        }
      }
    }
  }
  std::sort(layer_coord_list.begin(), layer_coord_list.end(), CmpLayerCoordByXASC());
  layer_coord_list.erase(std::unique(layer_coord_list.begin(), layer_coord_list.end()), layer_coord_list.end());

  std::vector<AccessPoint> access_point_list;
  for (LayerCoord& layer_coord : layer_coord_list) {
    access_point_list.emplace_back(layer_coord.get_x(), layer_coord.get_y(), layer_coord.get_layer_idx(), AccessPointType::kPrefTrackGrid);
  }
  return access_point_list;
}

std::vector<AccessPoint> PinAccessor::getAccessPointListByCurrTrackGrid(std::vector<LayerRect>& legal_shape_list)
{
  std::vector<RoutingLayer>& routing_layer_list = DM_INST.getDatabase().get_routing_layer_list();

  std::vector<LayerCoord> layer_coord_list;
  for (LayerRect& legal_shape : legal_shape_list) {
    irt_int lb_x = legal_shape.get_lb_x();
    irt_int lb_y = legal_shape.get_lb_y();
    irt_int rt_x = legal_shape.get_rt_x();
    irt_int rt_y = legal_shape.get_rt_y();
    irt_int curr_layer_idx = legal_shape.get_layer_idx();
    RoutingLayer curr_routing_layer = routing_layer_list[curr_layer_idx];
    // curr layer track grid
    for (irt_int x : RTUtil::getClosedScaleList(lb_x, rt_x, curr_routing_layer.getXTrackGridList())) {
      for (irt_int y : RTUtil::getClosedScaleList(lb_y, rt_y, curr_routing_layer.getYTrackGridList())) {
        layer_coord_list.emplace_back(x, y, curr_layer_idx);
      }
    }
  }
  std::sort(layer_coord_list.begin(), layer_coord_list.end(), CmpLayerCoordByXASC());
  layer_coord_list.erase(std::unique(layer_coord_list.begin(), layer_coord_list.end()), layer_coord_list.end());

  std::vector<AccessPoint> access_point_list;
  for (LayerCoord& layer_coord : layer_coord_list) {
    access_point_list.emplace_back(layer_coord.get_x(), layer_coord.get_y(), layer_coord.get_layer_idx(), AccessPointType::kCurrTrackGrid);
  }
  return access_point_list;
}

std::vector<AccessPoint> PinAccessor::getAccessPointListByOnTrack(std::vector<LayerRect>& legal_shape_list)
{
  std::vector<RoutingLayer>& routing_layer_list = DM_INST.getDatabase().get_routing_layer_list();

  std::vector<LayerCoord> layer_coord_list;
  for (LayerRect& legal_shape : legal_shape_list) {
    irt_int lb_x = legal_shape.get_lb_x();
    irt_int lb_y = legal_shape.get_lb_y();
    irt_int rt_x = legal_shape.get_rt_x();
    irt_int rt_y = legal_shape.get_rt_y();
    irt_int curr_layer_idx = legal_shape.get_layer_idx();
    RoutingLayer curr_routing_layer = routing_layer_list[curr_layer_idx];
    // on track
    irt_int mid_x = (lb_x + rt_x) / 2;
    irt_int mid_y = (lb_y + rt_y) / 2;
    for (irt_int x : RTUtil::getClosedScaleList(lb_x, rt_x, curr_routing_layer.getXTrackGridList())) {
      for (irt_int y : {lb_y, mid_y, rt_y}) {
        layer_coord_list.emplace_back(x, y, curr_layer_idx);
      }
    }
    for (irt_int y : RTUtil::getClosedScaleList(lb_y, rt_y, curr_routing_layer.getYTrackGridList())) {
      for (irt_int x : {lb_x, mid_x, rt_x}) {
        layer_coord_list.emplace_back(x, y, curr_layer_idx);
      }
    }
  }
  std::sort(layer_coord_list.begin(), layer_coord_list.end(), CmpLayerCoordByXASC());
  layer_coord_list.erase(std::unique(layer_coord_list.begin(), layer_coord_list.end()), layer_coord_list.end());

  std::vector<AccessPoint> access_point_list;
  for (LayerCoord& layer_coord : layer_coord_list) {
    access_point_list.emplace_back(layer_coord.get_x(), layer_coord.get_y(), layer_coord.get_layer_idx(), AccessPointType::kOnTrack);
  }
  return access_point_list;
}

std::vector<AccessPoint> PinAccessor::getAccessPointListByOnShape(std::vector<LayerRect>& legal_shape_list)
{
  std::vector<LayerCoord> layer_coord_list;
  for (LayerRect& legal_shape : legal_shape_list) {
    irt_int lb_x = legal_shape.get_lb_x();
    irt_int lb_y = legal_shape.get_lb_y();
    irt_int rt_x = legal_shape.get_rt_x();
    irt_int rt_y = legal_shape.get_rt_y();
    irt_int curr_layer_idx = legal_shape.get_layer_idx();
    // on shape
    irt_int mid_x = (lb_x + rt_x) / 2;
    irt_int mid_y = (lb_y + rt_y) / 2;
    for (irt_int x : {lb_x, mid_x, rt_x}) {
      for (irt_int y : {lb_y, mid_y, rt_y}) {
        layer_coord_list.emplace_back(x, y, curr_layer_idx);
      }
    }
  }
  std::sort(layer_coord_list.begin(), layer_coord_list.end(), CmpLayerCoordByXASC());
  layer_coord_list.erase(std::unique(layer_coord_list.begin(), layer_coord_list.end()), layer_coord_list.end());

  std::vector<AccessPoint> access_point_list;
  for (LayerCoord& layer_coord : layer_coord_list) {
    access_point_list.emplace_back(layer_coord.get_x(), layer_coord.get_y(), layer_coord.get_layer_idx(), AccessPointType::kOnShape);
  }
  return access_point_list;
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
  bounding_box.set_grid_rect(RTUtil::getOpenGCellGridRect(bounding_box.get_real_rect(), gcell_axis));
}

void PinAccessor::updateAccessGrid(PANet& pa_net)
{
  ScaleAxis& gcell_axis = DM_INST.getDatabase().get_gcell_axis();
  BoundingBox& bounding_box = pa_net.get_bounding_box();

  for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
    for (AccessPoint& access_point : pa_pin.get_access_point_list()) {
      access_point.set_grid_coord(RTUtil::getGCellGridCoordByBBox(access_point.get_real_coord(), gcell_axis, bounding_box));
    }
  }
}

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
    PAPin& pa_driving_pin = pa_net.get_pa_driving_pin();
    for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
      if (pa_driving_pin.get_pin_idx() != pa_pin.get_pin_idx()) {
        continue;
      }
      pa_driving_pin = pa_pin;
    }
    Pin& driving_pin = pa_net.get_origin_net()->get_driving_pin();
    if (driving_pin.get_pin_idx() != pa_driving_pin.get_pin_idx()) {
      LOG_INST.error(Loc::current(), "The pin idx is not equal!");
    }
    driving_pin.set_protected_access_point(pa_driving_pin.get_protected_access_point());
    driving_pin.set_access_point_list(pa_driving_pin.get_access_point_list());

    pa_net.get_origin_net()->set_bounding_box(pa_net.get_bounding_box());
  }
}

#endif

#if 1  // plot pa_model

void PinAccessor::plotPAModel(PAModel& pa_model, std::string flag)
{
  Die& die = DM_INST.getDatabase().get_die();
  std::vector<RoutingLayer>& routing_layer_list = DM_INST.getDatabase().get_routing_layer_list();
  GridMap<GCell>& gcell_map = DM_INST.getDatabase().get_gcell_map();
  std::string pa_temp_directory_path = DM_INST.getConfig().pa_temp_directory_path;

  GPGDS gp_gds;

  // track_axis_struct
  GPStruct track_axis_struct("track_axis_struct");
  for (RoutingLayer& routing_layer : routing_layer_list) {
    std::vector<irt_int> x_list = RTUtil::getClosedScaleList(die.get_real_lb_x(), die.get_real_rt_x(), routing_layer.getXTrackGridList());
    std::vector<irt_int> y_list = RTUtil::getClosedScaleList(die.get_real_lb_y(), die.get_real_rt_y(), routing_layer.getYTrackGridList());
    for (irt_int x : x_list) {
      GPPath gp_path;
      gp_path.set_data_type(static_cast<irt_int>(GPDataType::kAxis));
      gp_path.set_segment(x, die.get_real_lb_y(), x, die.get_real_rt_y());
      gp_path.set_layer_idx(GP_INST.getGDSIdxByRouting(routing_layer.get_layer_idx()));
      track_axis_struct.push(gp_path);
    }
    for (irt_int y : y_list) {
      GPPath gp_path;
      gp_path.set_data_type(static_cast<irt_int>(GPDataType::kAxis));
      gp_path.set_segment(die.get_real_lb_x(), y, die.get_real_rt_x(), y);
      gp_path.set_layer_idx(GP_INST.getGDSIdxByRouting(routing_layer.get_layer_idx()));
      track_axis_struct.push(gp_path);
    }
  }
  gp_gds.addStruct(track_axis_struct);

  // 整张版图的fixed_rect
  for (irt_int x = 0; x < gcell_map.get_x_size(); x++) {
    for (irt_int y = 0; y < gcell_map.get_y_size(); y++) {
      GCell& gcell = gcell_map[x][y];
      for (auto& [is_routing, layer_net_fixed_rect_map] : gcell.get_type_layer_net_fixed_rect_map()) {
        for (auto& [layer_idx, net_fixed_rect_map] : layer_net_fixed_rect_map) {
          for (auto& [net_idx, fixed_rect_set] : net_fixed_rect_map) {
            GPStruct fixed_rect_struct(RTUtil::getString("fixed_rect(net_", net_idx, ")"));
            for (auto& fixed_rect : fixed_rect_set) {
              GPBoundary gp_boundary;
              gp_boundary.set_data_type(static_cast<irt_int>(GPDataType::kShape));
              gp_boundary.set_rect(fixed_rect->get_real_rect());
              if (is_routing) {
                gp_boundary.set_layer_idx(GP_INST.getGDSIdxByRouting(layer_idx));
              } else {
                gp_boundary.set_layer_idx(GP_INST.getGDSIdxByCut(layer_idx));
              }
              fixed_rect_struct.push(gp_boundary);
            }
            gp_gds.addStruct(fixed_rect_struct);
          }
        }
      }
    }
  }

  // access_point
  for (PANet& pa_net : pa_model.get_pa_net_list()) {
    GPStruct access_point_struct(RTUtil::getString("access_point(net_", pa_net.get_net_idx(), ")"));
    for (PAPin& pa_pin : pa_net.get_pa_pin_list()) {
      for (AccessPoint& access_point : pa_pin.get_access_point_list()) {
        irt_int x = access_point.get_real_x();
        irt_int y = access_point.get_real_y();

        GPBoundary access_point_boundary;
        access_point_boundary.set_layer_idx(GP_INST.getGDSIdxByRouting(access_point.get_layer_idx()));
        access_point_boundary.set_data_type(static_cast<irt_int>(GPDataType::kAccessPoint));
        access_point_boundary.set_rect(x - 10, y - 10, x + 10, y + 10);
        access_point_struct.push(access_point_boundary);
      }
    }
    gp_gds.addStruct(access_point_struct);
  }

  std::string gds_file_path = RTUtil::getString(pa_temp_directory_path, flag, "_pa.gds");
  GP_INST.plot(gp_gds, gds_file_path);
}

#endif

}  // namespace irt
