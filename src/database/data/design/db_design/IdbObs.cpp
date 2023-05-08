/**
 * @project		iDB
 * @file		IdbObs.h
 * @date		25/05/2021
 * @version		0.1
* @description


        Describe Obstrction information,.
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "IdbObs.h"

using namespace std;
namespace idb {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IdbObsLayer::IdbObsLayer()
{
  _layer_shape = new IdbLayerShape();
}

IdbObsLayer::~IdbObsLayer()
{
  if (_layer_shape) {
    delete _layer_shape;
    _layer_shape = nullptr;
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IdbObs::IdbObs()
{
}

IdbObs::~IdbObs()
{
}

IdbObsLayer* IdbObs::add_obs_layer(IdbObsLayer* obs_layer)
{
  IdbObsLayer* pLayer = obs_layer;
  if (pLayer == nullptr) {
    pLayer = new IdbObsLayer();
  }
  _obs_layer_list.emplace_back(pLayer);

  return pLayer;
}

}  // namespace idb
