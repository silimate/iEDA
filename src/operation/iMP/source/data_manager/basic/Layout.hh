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
 * @Date: 2022-01-21 11:18:37
 * @LastEditTime: 2022-03-29 19:30:35
 * @LastEditors: S.J Chen
 * @Description:
 * @FilePath: /iEDA/src/imp/src/database/Layout.hh
 * Contact : https://github.com/sjchanson
 */

#ifndef IMP_LAYOUT_H
#define IMP_LAYOUT_H

#include <map>
#include <string>

#include "Cell.hh"
#include "Geometry.hh"
#include "Row.hh"

namespace imp {

class Layout
{
 public:
  Layout() = default;
  Layout(const Layout&) = delete;
  Layout(Layout&&) = delete;
  ~Layout();

  Layout& operator=(const Layout&) = delete;
  Layout& operator=(Layout&&) = delete;

  // getter.
  int32_t get_database_unit() const { return _database_unit; }

  const geo::box<int32_t>& get_die_shape() const { return _die_shape; }
  geo::box<int32_t>& get_die_shape() { return _die_shape; }
  int32_t get_die_width() { return geo::width(_die_shape); }
  int32_t get_die_height() { return geo::height(_die_shape); }

  const geo::box<int32_t>& get_core_shape() const { return _core_shape; }
  geo::box<int32_t>& get_core_shape() { return _core_shape; }
  int32_t get_core_width() { return geo::width(_core_shape); }
  int32_t get_core_height() { return geo::height(_core_shape); }

  int32_t get_row_height() const { return _row_list[0]->get_site_height(); }
  int32_t get_site_width() const { return _row_list[0]->get_site_width(); }

  const std::vector<std::shared_ptr<Row>> get_row_list() const { return _row_list; }
  const std::vector<std::shared_ptr<Cell>> get_cell_list() const { return _cell_list; }

  // setter.
  void set_database_unit(int32_t dbu) { _database_unit = dbu; }
  void set_die_shape(geo::box<int32_t> rect) { _die_shape = std::move(rect); }
  void set_core_shape(geo::box<int32_t> rect) { _core_shape = std::move(rect); }

  void add_row(std::shared_ptr<Row> row);
  void add_cell(std::shared_ptr<Cell> cell);

  // function.
  std::shared_ptr<Row> find_row(const std::string& row_name) const;
  std::shared_ptr<Cell> find_cell(const std::string& cell_name) const;

 private:
  int32_t _database_unit = -1;
  geo::box<int32_t> _die_shape;
  geo::box<int32_t> _core_shape;

  std::vector<std::shared_ptr<Row>> _row_list;
  std::vector<std::shared_ptr<Cell>> _cell_list;

  std::map<std::string, std::shared_ptr<Row>> _name_to_row_map;
  std::map<std::string, std::shared_ptr<Cell>> _name_to_cell_map;
};

inline Layout::~Layout()
{
  _cell_list.clear();
  _name_to_cell_map.clear();
}

inline void Layout::add_row(std::shared_ptr<Row> row)
{
  _row_list.push_back(row);
  _name_to_row_map.emplace(row->get_name(), row);
}

inline void Layout::add_cell(std::shared_ptr<Cell> cell)
{
  _cell_list.push_back(cell);
  _name_to_cell_map.emplace(cell->get_name(), cell);
}

inline std::shared_ptr<Row> Layout::find_row(const std::string& row_name) const
{
  std::shared_ptr<Row> row = nullptr;
  auto row_iter = _name_to_row_map.find(row_name);
  if (row_iter != _name_to_row_map.end()) {
    row = (*row_iter).second;
  }
  return row;
}

inline std::shared_ptr<Cell> Layout::find_cell(const std::string& cell_name) const
{
  std::shared_ptr<Cell> cell = nullptr;
  auto cell_iter = _name_to_cell_map.find(cell_name);
  if (cell_iter != _name_to_cell_map.end()) {
    cell = (*cell_iter).second;
  }
  return cell;
}

}  // namespace imp

#endif