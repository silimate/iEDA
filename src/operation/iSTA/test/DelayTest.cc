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
#include <iostream>
#include <string>
#include <utility>

#include "delay/ElmoreDelayCalc.hh"
#include "gtest/gtest.h"
#include "liberty/Liberty.hh"
#include "log/Log.hh"
#include "netlist/Net.hh"
#include "netlist/Netlist.hh"
#include "sta/Sta.hh"

using ieda::Log;
using ista::DesignObject;
using ista::Liberty;
using ista::Net;
using ista::NetIterator;
using ista::Netlist;
using ista::NetPinIterator;
using ista::RcNet;
using ista::Sta;

namespace {

class DelayTest : public testing::Test {
  void SetUp() {
    char config[] = "test";
    char* argv[] = {config};
    Log::init(argv);
  }
  void TearDown() { Log::end(); }
};

}  // namespace