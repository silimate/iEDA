include_directories(
    /opt/homebrew/include
    /opt/homebrew/opt/tcl-tk/include
    /opt/homebrew/opt/libomp/include
    /opt/homebrew/opt/eigen/include
    /usr/local/include
)
link_directories(
    /opt/homebrew/lib
    /opt/homebrew/opt/tcl-tk/lib
    /opt/homebrew/opt/libomp/lib
    /usr/local/lib
)

set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp")
set(OpenMP_C_LIB_NAMES "omp")
set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
set(OpenMP_CXX_LIB_NAMES "omp")
set(OpenMP_omp_LIBRARY "omp")
