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

#ifndef IMP_BLOCK_H
#define IMP_BLOCK_H
#include <future>
#include <ranges>

#include "Netlist.hh"
#include "Object.hh"
// #include "ShapeCurve.hh"
namespace imp {
class Block;

template <typename Operator>
concept BlockOperator = requires { std::is_invocable_v<Operator, Block>; };

class Block : public Object, public std::enable_shared_from_this<Block>
{
 public:
  Block() = default;
  Block(std::string name, std::shared_ptr<Netlist> netlist = nullptr, std::shared_ptr<Object> parent = nullptr);
  ~Block() = default;

  virtual OBJ_TYPE object_type() const override { return OBJ_TYPE::kBlock; }
  virtual geo::box<int32_t> boundingbox() const override { return Object::transform(_shape); }
  void set_shape(const geo::box<int32_t>& box)
  {
    set_min_corner(geo::lx(box), geo::ly(box));
    _shape = geo::make_box(0, 0, geo::width(box), geo::height(box));
  }
  size_t level() const;
  bool is_leaf();
  Netlist& netlist();
  const Netlist& netlist() const;
  void set_netlist(std::shared_ptr<Netlist> netlist);
  template <BlockOperator Operator>
  void preorder_op(Operator op);
/**
 * @brief Parallel preorder 
 * 
 * @tparam Operator 
 * @param op 
 */
  template <BlockOperator Operator>
  void parallel_preorder_op(Operator op);

  template <BlockOperator Operator>
  void postorder_op(Operator op);

  template <BlockOperator Operator>
  void level_order_op(Operator op);
  using std::enable_shared_from_this<Block>::shared_from_this;

 private:
  geo::box<int32_t> _shape;

  std::shared_ptr<Netlist> _netlist;
};

template <BlockOperator Operator>
inline void Block::preorder_op(Operator op)
{
  op(*this);
  for (auto&& i : _netlist->vRange()) {
    auto obj = i.property();
    if (!obj->isBlock())
      continue;
    auto sub_block = std::static_pointer_cast<Block, Object>(obj);
    sub_block->preorder_op(op);
  }
}

template <BlockOperator Operator>
inline void Block::postorder_op(Operator op)
{
  for (auto&& i : _netlist->vRange()) {
    auto obj = i.property();
    if (!obj->isBlock())
      continue;
    auto sub_block = std::static_pointer_cast<Block, Object>(obj);
    sub_block->preorder_op(op);
  }
  op(*this);
}

template <BlockOperator Operator>
void Block::parallel_preorder_op(Operator op)
{
  op(*this);

  std::vector<std::future<void>> futures;
  for (auto&& i : _netlist->vRange()) {
    auto obj = i.property();
    if (!obj->isBlock())
      continue;

    auto sub_block = std::static_pointer_cast<Block, Object>(obj);
    futures.push_back(std::async(std::launch::async, &Block::preorder_op<Operator>, sub_block, op));
  }

  for (auto&& future : futures) {
    future.get();
  }
}
}  // namespace imp

#endif