#pragma once
/**
 * iEDA
 * Copyright (C) 2021  PCL
 *
 * This program is free software;
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @project		iDB
 * @file		IdbDesign.h
 * @copyright	(c) 2021 All Rights Reserved.
 * @date		25/05/2021
 * @version		0.1
 * @description


        Describe lef .
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "db_design/IdbTrackGrid.h"
#include "db_design/IdbVias.h"
#include "db_layout/IdbCellMaster.h"
#include "db_layout/IdbCore.h"
#include "db_layout/IdbDie.h"
#include "db_layout/IdbGCellGrid.h"
#include "db_layout/IdbLayer.h"
#include "db_layout/IdbRow.h"
#include "db_layout/IdbSite.h"
#include "db_layout/IdbTerm.h"
#include "db_layout/IdbUnits.h"
#include "db_layout/IdbViaMaster.h"
#include "db_layout/IdbViaRule.h"

namespace idb {

class IdbLayout
{
 public:
  IdbLayout();
  ~IdbLayout();

  // getter
  int32_t get_munufacture_grid() { return _manufacture_grid; }
  IdbDie* get_die() { return _die; }
  IdbCore* get_core();
  IdbUnits* get_units() { return _units; }
  IdbLayers* get_layers() { return _layers; }
  IdbSites* get_sites() { return _sites; }
  IdbRows* get_rows() { return _rows; }
  IdbGCellGridList* get_gcell_grid_list() { return _gcell_grid_list; }
  IdbTrackGridList* get_track_grid_list() { return _track_grid_list; }
  // IdbMacros* get_macros(){return _macros;}
  IdbCellMasterList* get_cell_master_list() { return _cell_master_list; }
  IdbVias* get_via_list() { return _via_list; }
  IdbViaRuleList* get_via_rule_list() { return _via_rule_list; }

  // setter
  void set_manufacture_grid(int32_t value) { _manufacture_grid = value; }
  void set_die(IdbDie* die) { _die = die; }
  void set_units(IdbUnits* units) { _units = units; }
  void set_layer(IdbLayers* layers) { _layers = layers; }
  void set_sites(IdbSites* sites) { _sites = sites; }
  void set_rows(IdbRows* rows) { _rows = rows; }
  void set_gcell_grid_list(IdbGCellGridList* gcell_grid_list) { _gcell_grid_list = gcell_grid_list; }
  void set_track_grid_list(IdbTrackGridList* track_grid_list) { _track_grid_list = track_grid_list; }
  // void set_macros(IdbMacros* macros){_macros = macros;}
  void set_cell_master_list(IdbCellMasterList* master_list) { _cell_master_list = master_list; }
  void set_via_list(IdbVias* via_list) { _via_list = via_list; }
  void set_via_rule_list(IdbViaRuleList* via_rule_list) { _via_rule_list = via_rule_list; }
  // operator
  int32_t transAreaDB(double value) { return std::round(std::pow(_units->get_micron_dbu(), 2) * value); }
  int32_t transUnitDB(double value) { return std::round(_units->get_micron_dbu() * value); }

 private:
  int32_t _manufacture_grid;  //<---------tbd---------------
  IdbUnits* _units;
  IdbDie* _die;
  IdbCore* _core;
  IdbLayers* _layers;
  IdbSites* _sites;
  IdbRows* _rows;
  IdbGCellGridList* _gcell_grid_list;
  IdbTrackGridList* _track_grid_list;
  IdbCellMasterList* _cell_master_list;
  IdbVias* _via_list;
  IdbViaRuleList* _via_rule_list;
};

}  // namespace idb
