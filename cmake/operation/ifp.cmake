SET(DB_ROOT ${HOME_DATABASE}/iDB)
SET(FP_ROOT ${HOME_OPERATION}/iFP)

enable_testing()
include_directories(${DB_ROOT})
include_directories(${FP_ROOT}/include)
include_directories(${FP_ROOT}/interface)
include_directories(${FP_ROOT}/logger)
include_directories(${FP_ROOT}/include/fp_db)
include_directories(${FP_ROOT}/IOPlacer)
include_directories(${FP_ROOT}/database)
include_directories(${FP_ROOT}/Wrapper)
include_directories(${FP_ROOT}/MacroPlacer)
include_directories(${FP_ROOT}/TapCell)
include_directories(${FP_ROOT}/Pdn)
include_directories(${FP_ROOT}/Tcl)
include_directories(${HOME_UTILITY}/stdBase/include)
include_directories(${HOME_UTILITY})
include_directories(${HOME_UTILITY}/string)
include_directories(${HOME_UTILITY}/tcl)
include_directories(${HOME_UTILITY}/log)
include_directories(SYSTEM ${HOME_THIRDPARTY})

link_directories(${FP_ROOT}/libs)
