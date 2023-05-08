#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace igui {
  enum class GuiTreeColEnum : int32_t { kLayerColor, kName, kVisible, kMax };

  enum class GuiOptionValue : int32_t { kOff, kOn, kmax };

  struct GuiOption {
    string _name;
    GuiOptionValue _visible;
  };

  class GuiTreeNode {
   public:
    GuiTreeNode() { }
    GuiTreeNode(const string node_name) : _node_name(node_name) { }
    ~GuiTreeNode() = default;

    /// getter
    string get_node_name() { return _node_name; }
    vector<GuiOption>& get_option_list() { return _option_list; }

    /// setter
    void set_node_name(string node_name) { _node_name = node_name; }
    void add_option(GuiOption option) { _option_list.push_back(option); }
    void add_option(string name, GuiOptionValue visible = GuiOptionValue::kOff) {
      GuiOption option = { name, visible };
      _option_list.push_back(option);
    }

    void set_option(string option_name, bool b_value) {
      auto result = std::find_if(_option_list.begin(), _option_list.end(),
                                 [option_name](const auto& iter) { return iter._name == option_name; });

      if (result != _option_list.end()) {
        result->_visible = b_value ? GuiOptionValue::kOn : GuiOptionValue::kOff;
      }

      cout << _node_name << "->" << option_name << " = " << b_value << endl;
    }

    void set_option_list(bool b_value) {
      for (auto& option : _option_list) {
        option._visible = b_value ? GuiOptionValue::kOn : GuiOptionValue::kOff;
        cout << _node_name << "->" << option._name << " = " << b_value << endl;
      }
    }

    void clear() { _option_list.clear(); }

    /// operator
    bool isChecked(string option_name) {
      auto result = std::find_if(_option_list.begin(), _option_list.end(),
                                 [option_name](const auto& iter) { return iter._name == option_name; });

      if (result != _option_list.end()) {
        return result->_visible == GuiOptionValue::kOn ? true : false;
      }

      return false;
    }

    bool isChecked(int32_t index) {
      if (index < (int32_t)_option_list.size()) {
        return _option_list[index]._visible == GuiOptionValue::kOn ? true : false;
      }

      return false;
    }

    bool isLayerList() { return _node_name == "Layer" ? true : false; }

   private:
    string _node_name;
    vector<GuiOption> _option_list;
  };

}  // namespace igui
