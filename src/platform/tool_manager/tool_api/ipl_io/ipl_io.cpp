#include "ipl_io.h"

#include "builder.h"
#include "flow_config.h"
#include "iPL_API.hh"
#include "idm.h"

namespace iplf {
PlacerIO* PlacerIO::_instance = nullptr;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlacerIO::initPlacer(std::string config)
{
  if (config.empty()) {
    /// set config path
    config = flowConfigInst->get_ipl_path();
  }
  flowConfigInst->set_status_stage("iPL - Placement");
  iPLAPIInst.initAPI(config, dmInst->get_idb_builder());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlacerIO::destroyPlacer()
{
  if (iPLAPIInst.isPlacerDBStarted()) {
    iPLAPIInst.destoryInst();
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PlacerIO::runPlacement()
{
  if (!iPLAPIInst.isPlacerDBStarted()) {
    this->initPlacer("");
  } else {
    iPLAPIInst.updatePlacerDB();
  }

  ieda::Stats stats;
  iPLAPIInst.runFlow();

  flowConfigInst->add_status_runtime(stats.elapsedRunTime());
  flowConfigInst->set_status_memmory(stats.memoryDelta());

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PlacerIO::runIncrementalLegalization()
{
  if (!iPLAPIInst.isPlacerDBStarted()) {
    this->initPlacer("");
  } else {
    iPLAPIInst.updatePlacerDB();
  }

  bool flag = true;
  if (!iPLAPIInst.isAbucasLGStarted()) {
    flag = iPLAPIInst.runLG();
  } else {
    flag = iPLAPIInst.runIncrLG();
  }

  return flag;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PlacerIO::runIncrementalLegalization(std::vector<std::string>& changed_inst_list)
{
  if (!iPLAPIInst.isPlacerDBStarted()) {
    this->initPlacer("");
  } else {
    iPLAPIInst.updatePlacerDB(changed_inst_list);
  }

  bool flag = true;
  if (!iPLAPIInst.isAbucasLGStarted()) {
    flag = iPLAPIInst.runLG();
  } else {
    flag = iPLAPIInst.runIncrLG(changed_inst_list);
  }

  return flag;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PlacerIO::runFillerInsertion(std::string config)
{
  if (config.empty()) {
    /// set config path
    config = flowConfigInst->get_ipl_path();
  }

  flowConfigInst->set_status_stage("iPL - Filler");

  ieda::Stats stats;

  iPLAPIInst.initAPI(config, dmInst->get_idb_builder());

  iPLAPIInst.insertLayoutFiller();

  iPLAPIInst.destoryInst();

  flowConfigInst->add_status_runtime(stats.elapsedRunTime());
  flowConfigInst->set_status_memmory(stats.memoryDelta());

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PlacerIO::runMacroPlacement()
{
  if (!iPLAPIInst.isPlacerDBStarted()) {
    this->initPlacer("");
  } else {
    iPLAPIInst.updatePlacerDB();
  }

  iPLAPIInst.runMP();

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PlacerIO::runGlobalPlacement()
{
  if (!iPLAPIInst.isPlacerDBStarted()) {
    this->initPlacer("");
  } else {
    iPLAPIInst.updatePlacerDB();
  }

  iPLAPIInst.runGP();

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PlacerIO::runLegalization()
{
  if (!iPLAPIInst.isPlacerDBStarted()) {
    this->initPlacer("");
  } else {
    iPLAPIInst.updatePlacerDB();
  }

  bool flag = iPLAPIInst.runLG();

  return flag;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlacerIO::runDetailPlacement()
{
  if (!iPLAPIInst.isPlacerDBStarted()) {
    this->initPlacer("");
  } else {
    iPLAPIInst.updatePlacerDB();
  }

  iPLAPIInst.runDP();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PlacerIO::checkLegality()
{
  if (!iPLAPIInst.isPlacerDBStarted()) {
    this->initPlacer("");
  } else {
    iPLAPIInst.updatePlacerDB();
  }

  bool flag = iPLAPIInst.checkLegality();
  return flag;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PlacerIO::reportPlacement()
{
  if (!iPLAPIInst.isPlacerDBStarted()) {
    this->initPlacer("");
  } else {
    iPLAPIInst.updatePlacerDB();
  }

  // TODO

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PlacerIO::runIncrementalFlow(std::string config)
{
  if (config.empty()) {
    /// set config path
    config = flowConfigInst->get_ipl_path();
  }

  flowConfigInst->set_status_stage("iPL - Legalization");

  ieda::Stats stats;

  iPLAPIInst.initAPI(config, dmInst->get_idb_builder());

  iPLAPIInst.runIncrementalFlow();

  iPLAPIInst.destoryInst();

  flowConfigInst->add_status_runtime(stats.elapsedRunTime());
  flowConfigInst->set_status_memmory(stats.memoryDelta());

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int64_t PlacerIO::getFileInstanceBufferSize()
{
  int64_t buffer_size = sizeof(FileInstanceHeader);
  for (auto& file_inst : _file_inst_list) {
    buffer_size = buffer_size + sizeof(file_inst);
  }
  return buffer_size;
}

bool PlacerIO::readInstanceDataFromFile(std::string path)
{
  if (path.empty()) {
    return false;
  }

  FilePlacementManager file(path, (int32_t) PlDbId::kPlInstanceData);

  return file.readFile();
}

bool PlacerIO::saveInstanceDataToFile(std::string path)
{
  if (path.empty()) {
    return false;
  }

  FilePlacementManager file(path, (int32_t) PlDbId::kPlInstanceData);

  return file.writeFile();
}

bool PlacerIO::readInstanceDataFromDirectory(std::string directory)
{
  if (directory.empty()) {
    directory = _directory;
    if (directory.empty()) {
      return false;
    }
  } else {
    _directory = directory;
  }

  std::string path = directory + "pl_iter_" + std::to_string(_dp_index++) + ".pl";

  return readInstanceDataFromFile(path);
}

bool PlacerIO::saveInstanceDataToDirectory(std::string directory)
{
  if (directory.empty()) {
    return false;
  }

  std::string path = directory + "pl_iter_" + std::to_string(_dp_index++) + ".pl";

  return saveInstanceDataToFile(path);
}

}  // namespace iplf
