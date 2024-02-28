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

#include "PlanarRect.hpp"

namespace irt {

class EXTPlanarRect
{
 public:
  EXTPlanarRect() = default;
  ~EXTPlanarRect() = default;
  // getter
  PlanarRect& get_grid_rect() { return _grid_rect; }
  PlanarCoord& get_grid_lb() { return _grid_rect.get_lb(); }
  PlanarCoord& get_grid_rt() { return _grid_rect.get_rt(); }
  int32_t get_grid_lb_x() const { return _grid_rect.get_lb_x(); }
  int32_t get_grid_lb_y() const { return _grid_rect.get_lb_y(); }
  int32_t get_grid_rt_x() const { return _grid_rect.get_rt_x(); }
  int32_t get_grid_rt_y() const { return _grid_rect.get_rt_y(); }
  PlanarRect& get_real_rect() { return _real_rect; }
  PlanarCoord& get_real_lb() { return _real_rect.get_lb(); }
  PlanarCoord& get_real_rt() { return _real_rect.get_rt(); }
  int32_t get_real_lb_x() const { return _real_rect.get_lb_x(); }
  int32_t get_real_lb_y() const { return _real_rect.get_lb_y(); }
  int32_t get_real_rt_x() const { return _real_rect.get_rt_x(); }
  int32_t get_real_rt_y() const { return _real_rect.get_rt_y(); }
  // const getter
  const PlanarRect& get_grid_rect() const { return _grid_rect; }
  const PlanarRect& get_real_rect() const { return _real_rect; }
  // setter
  void set_grid_rect(const PlanarRect& grid_rect) { _grid_rect = grid_rect; }
  void set_grid_lb(const PlanarCoord& grid_lb) { _grid_rect.set_lb(grid_lb); }
  void set_grid_rt(const PlanarCoord& grid_rt) { _grid_rect.set_rt(grid_rt); }
  void set_grid_lb(const int32_t x, const int32_t y) { _grid_rect.set_lb(x, y); }
  void set_grid_rt(const int32_t x, const int32_t y) { _grid_rect.set_rt(x, y); }
  void set_grid_lb_x(const int32_t x) { _grid_rect.set_lb_x(x); }
  void set_grid_rt_x(const int32_t x) { _grid_rect.set_rt_x(x); }
  void set_grid_lb_y(const int32_t y) { _grid_rect.set_lb_y(y); }
  void set_grid_rt_y(const int32_t y) { _grid_rect.set_rt_y(y); }
  void set_real_rect(const PlanarRect& real_rect) { _real_rect = real_rect; }
  void set_real_lb(const PlanarCoord& real_lb) { _real_rect.set_lb(real_lb); }
  void set_real_rt(const PlanarCoord& real_rt) { _real_rect.set_rt(real_rt); }
  void set_real_lb(const int32_t x, const int32_t y) { _real_rect.set_lb(x, y); }
  void set_real_rt(const int32_t x, const int32_t y) { _real_rect.set_rt(x, y); }
  void set_real_lb_x(const int32_t x) { _real_rect.set_lb_x(x); }
  void set_real_rt_x(const int32_t x) { _real_rect.set_rt_x(x); }
  void set_real_lb_y(const int32_t y) { _real_rect.set_lb_y(y); }
  void set_real_rt_y(const int32_t y) { _real_rect.set_rt_y(y); }
  // function
  inline int32_t getXSize() const;
  inline int32_t getYSize() const;
  inline int32_t getTotalSize() const;
  inline int32_t getRealLength() const;
  inline int32_t getRealWidth() const;
  inline int32_t getRealHalfPerimeter() const;
  inline int32_t getRealPerimeter() const;
  inline int32_t getRealArea() const;

 private:
  PlanarRect _grid_rect;
  PlanarRect _real_rect;
};

inline int32_t EXTPlanarRect::getXSize() const
{
  return get_grid_rt_x() - get_grid_lb_x() + 1;
}

inline int32_t EXTPlanarRect::getYSize() const
{
  return get_grid_rt_y() - get_grid_lb_y() + 1;
}

inline int32_t EXTPlanarRect::getTotalSize() const
{
  return getXSize() * getYSize();
}

inline int32_t EXTPlanarRect::getRealLength() const
{
  return get_real_rt_x() - get_real_lb_x();
}

inline int32_t EXTPlanarRect::getRealWidth() const
{
  return get_real_rt_y() - get_real_lb_y();
}

inline int32_t EXTPlanarRect::getRealHalfPerimeter() const
{
  return getRealLength() + getRealWidth();
}

inline int32_t EXTPlanarRect::getRealPerimeter() const
{
  return getRealHalfPerimeter() * 2;
}

inline int32_t EXTPlanarRect::getRealArea() const
{
  return getRealLength() * getRealWidth();
}

}  // namespace irt
