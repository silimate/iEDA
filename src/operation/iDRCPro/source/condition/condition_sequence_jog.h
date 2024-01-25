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

#include <stdint.h>

#include <vector>

#include "condition_sequence.h"
#include "drc_basic_point.h"

namespace idrc {

class ConditionSequenceJog : public ConditionSequence
{
 public:
  ConditionSequenceJog(int filter_value, uint64_t trigger_sequence, uint64_t middle_sequence, uint64_t success_sequence)
      : ConditionSequence(filter_value, trigger_sequence), _middle_sequence(middle_sequence), _success_sequence(success_sequence)
  {
  }
  ~ConditionSequenceJog() override {}

  void applySequence(State& state, ConditionSequence::SequenceType sequence) override
  {
    switch (state) {
      case ConditionSequence::State::kNone:
        if (sequence & _trigger_sequence) {
          state = ConditionSequence::State::kTrigger;
          return;
        } else {
          state = ConditionSequence::State::kFail;
          return;
        }
        break;
      case ConditionSequence::State::kTrigger:
        if (sequence & _middle_sequence) {
          state = ConditionSequence::State::kRecording;
          return;
        } else {
          state = ConditionSequence::State::kFail;
          return;
        }
        break;
      case ConditionSequence::State::kRecording:
        if (sequence & _success_sequence) {
          state = ConditionSequence::State::kSuccess;
          return;
        } else if (sequence & _middle_sequence) {
          state = ConditionSequence::State::kRecording;
          return;
        } else {
          state = ConditionSequence::State::kFail;
          return;
        }
        break;
      case ConditionSequence::State::kSuccess:
        break;
      case ConditionSequence::State::kFail:
        break;
      default:
        break;
    }
  }

  void applyValue(State& state, SequenceType condition_sequence_enum, std::vector<DrcBasicPoint*> points) override
  {
    // TODO: calculate within and compare it to _filter_value, if fail change state to kFail
    // TODO: use sequence type to deside points to calculate within
  }

 private:
  uint64_t _middle_sequence;
  uint64_t _success_sequence;
};

}  // namespace idrc