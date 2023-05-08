/**
 * @project		iDB
 * @file		IdbTracks.h
 * @date		25/05/2021
 * @version		0.1
* @description


        Describe Tracks information,.
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "IdbTrackGrid.h"

namespace idb {

IdbTrack::IdbTrack()
{
  _direction = IdbTrackDirection::kNone;
  _pitch = 0;
  _width = 0;

  //_layer = nullptr;
  // _wires = nullptr;
  // _vias = nullptr;
}

IdbTrack::IdbTrack(IdbTrackDirection dir, uint32_t pitch, uint32_t width)
{
  _direction = dir;
  _pitch = pitch;
  _width = width;

  //_layer = nullptr;
  // _wires = nullptr;
  // _vias = nullptr;
}

IdbTrack::~IdbTrack()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IdbTrackGrid::IdbTrackGrid()
{
  _track = new IdbTrack();
  _track_num = 0;
}

IdbTrackGrid::~IdbTrackGrid()
{
  delete _track;
  _track = nullptr;
}

void IdbTrackGrid::add_layer_list(IdbLayer* layer)
{
  _layer_list.emplace_back(layer);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IdbTrackGridList::IdbTrackGridList()
{
  _track_grid_num = 0;
  _track_grid_list.clear();
}

IdbTrackGridList::~IdbTrackGridList()
{
  reset();
}

IdbTrackGrid* IdbTrackGridList::add_track_grid(IdbTrackGrid* track_grid)
{
  IdbTrackGrid* pGrid = track_grid;
  if (pGrid == nullptr) {
    pGrid = new IdbTrackGrid();
  }
  _track_grid_list.emplace_back(pGrid);
  _track_grid_num++;

  return pGrid;
}

void IdbTrackGridList::reset()
{
  for (auto& tg : _track_grid_list) {
    if (nullptr != tg) {
      delete tg;
      tg = nullptr;
    }
  }
  _track_grid_list.clear();

  _track_grid_num = 0;
}

}  // namespace idb
