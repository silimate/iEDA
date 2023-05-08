/**
 * @file IdbBus.cpp
 * @author pengming
 * @brief
 * @version 0.1
 * @date 2022-09-15
 */
#include "IdbBus.h"

#include <optional>
#include <sstream>

#include "IdbNet.h"

namespace idb {

std::optional<IdbBus> IdbBus::parseBusObj(const std::string& name_str, const IdbBusBitChars* bus_bit_chars)
{
  auto name_index = parseBusName(name_str, *bus_bit_chars);
  if (not name_index) {
    return std::nullopt;
  }
  IdbBus bus_obj;
  bus_obj.set_name(name_index->first);
  bus_obj.updateRange(name_index->second);
  return bus_obj;
}

void IdbBus::updateRange(unsigned index)
{
  _right = std::min(_right, index);
  _left = std::max(_left, index);
}
void IdbBus ::updateRange(const IdbBus& idb_bus_object)
{
  _right = std::min(_right, idb_bus_object._right);
  _left = std::max(_left, idb_bus_object._left);
}

IdbNet* IdbBus ::getNet(unsigned index)
{
  if (index >= _nets.size()) {
    return nullptr;
  }
  return _nets[index];
}

std::optional<std::reference_wrapper<IdbBus>> IdbBusList::findBus(const std::string& instance_name, const std::string& pin_name)
{
  return findBus(instance_name + "/" + pin_name);
}

// IdbBusList method
std::optional<std::reference_wrapper<IdbBus>> IdbBusList::findBus(const std::string& full_name)
{
  auto it = _bus_map.find(full_name);
  if (it == _bus_map.end()) {
    return std::nullopt;
  }
  return _bus_arr[it->second];
}

void IdbBusList::addBusObject(IdbBus&& idb_bus_object)
{
  auto it = _bus_map.find(idb_bus_object.get_name());
  if (it == _bus_map.end()) {
    size_t index = _bus_arr.size();
    _bus_map[idb_bus_object.get_name()] = index;
    _bus_arr.emplace_back(std::move(idb_bus_object));
  } else {
    _bus_arr[it->second].updateRange(idb_bus_object);
  }
}

std::optional<std::pair<std::string, unsigned>> IdbBus::parseBusName(const std::string& name_str, const IdbBusBitChars& bus_bit_chars)
{
  /**
   * @brief
   * FSM to parse bus name.
   * parse state:
   *  read [bus_index], skip escaped busbitchars \[\]
   */
  enum ParseState
  {
    ordinary,
    escape,
    busBitChar
  };
  std::stringstream ss;
  ParseState state = ordinary;
  bool is_bus = false;
  unsigned index = 0;
  for (auto c : name_str) {
    switch (state) {
      case ordinary:
        if (c == '\\') {
          state = escape;
        } else if (c == bus_bit_chars.getLeftDelimiter()) {
          state = busBitChar;
        } else {
          ss << c;
        }
        break;
      case escape:
        state = ordinary;
        ss << c;
        break;
      case busBitChar:
        if (c == bus_bit_chars.getRightDelimiter()) {
          state = ordinary;
          is_bus = true;
        } else {
          index = index * 10 + (c - '0');
        }
        break;
    }
  }
  if (state != ordinary || !is_bus) {
    return std::nullopt;
  }
  return std::pair<std::string, unsigned>{ss.str(), index};
}
void IdbBusList::addOrUpdate(const std::pair<std::string, unsigned>& info, const std::function<void(IdbBus&)>& setter)
{
  addOrUpdate(info.first, info.second, setter);
}

void IdbBusList::addOrUpdate(const std::string& name, unsigned index, const std::function<void(IdbBus&)>& setter)
{
  auto it = _bus_map.find(name);
  if (it == _bus_map.end()) {
    IdbBus bus(name, index, index);
    setter(bus);
    size_t arr_index = _bus_arr.size();
    _bus_arr.emplace_back(std::move(bus));
    _bus_map[name] = arr_index;
  } else {
    size_t arr_index = it->second;
    _bus_arr[arr_index].updateRange(index);
    setter(_bus_arr[arr_index]);
  }
}

void IdbBus::addPin(IdbPin* pin, unsigned index)
{
  if (_pins.size() <= index) {
    // resize: if capacity*2 < new_size, capacity will be set as new_size,
    // otherwise capacity will be doubled.
    _pins.resize(index + 1, nullptr);
  }
  _pins[index] = pin;
}

void IdbBus::addPin(IdbPin* pin)
{
  auto info = parseBusName(pin->get_pin_name(), IdbBusBitChars{});
  assert(info);
  unsigned index = info.value().second;
  addPin(pin, index);
}

void IdbBus::addNet(IdbNet* net, unsigned index)
{
  if (_nets.size() <= index) {
    _nets.resize(index + 1, nullptr);
  }
  _nets[index] = net;
}

void IdbBus::addNet(IdbNet* net)
{
  auto info = parseBusName(net->get_net_name(), IdbBusBitChars{});
  assert(info);
  unsigned index = info.value().second;
  addNet(net, index);
}

}  // namespace idb