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

#include <assert.h>
#include <libgen.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>
#include <any>
#include <array>
#include <boost/foreach.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/polygon/polygon.hpp>
#include <cassert>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <experimental/source_location>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <ostream>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>

#include "libfort/fort.hpp"
#include "omp.h"

using irt_int = int32_t;
#define DBL_ERROR 1E-5

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using GTLPointInt = gtl::point_data<irt_int>;
using GTLRectInt = gtl::rectangle_data<irt_int>;
using GTLPolyInt = gtl::polygon_90_data<irt_int>;
using GTLPolySetInt = gtl::polygon_90_set_data<irt_int>;

using BGPointInt = bg::model::d2::point_xy<double>;
using BGMultiPointInt = bg::model::multi_point<BGPointInt>;
using BGSegmentInt = bg::model::segment<BGPointInt>;
using BGLineInt = bg::model::linestring<BGPointInt>;
using BGMultiLineInt = bg::model::multi_linestring<BGLineInt>;
using BGRectInt = bg::model::box<BGPointInt>;
using BGPolyInt = bg::model::polygon<BGPointInt>;
using BGMultiPolyInt = bg::model::multi_polygon<BGPolyInt>;

using BGPointDBL = bg::model::d2::point_xy<double>;
using BGMultiPointDBL = bg::model::multi_point<BGPointDBL>;
using BGSegmentDBL = bg::model::segment<BGPointDBL>;
using BGLineDBL = bg::model::linestring<BGPointDBL>;
using BGMultiLineDBL = bg::model::multi_linestring<BGLineDBL>;
using BGRectDBL = bg::model::box<BGPointDBL>;
using BGPolyDBL = bg::model::polygon<BGPointDBL>;
using BGMultiPolyDBL = bg::model::multi_polygon<BGPolyDBL>;

template <class... Fs>
struct Overload : Fs...
{
  using Fs::operator()...;
};
template <class... Fs>
Overload(Fs...) -> Overload<Fs...>;
