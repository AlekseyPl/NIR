cmake_minimum_required(VERSION 3.10)
project(math CXX C ASM)

set(SOURCES
./ComplexMath.cpp
./DSPF_sp_fftSPxSP_cn.cpp
./DSPF_sp_ifftSPxSP_cn.cpp
./FftSP.cpp
./Peak.cpp
./Util.cpp
./C6xSim/C6xSimulator.c
./C6xSim/c66_data_sim.c
./C6xSim/c66_ag_intrins.c
)

set(HEADERS
./Complex.h
./IntComplex.h
./ComplexMath.h
./DSPF_sp_fftSPxSP_cn.h
./DSPF_sp_ifftSPxSP_cn.h
./FftSP.h
./Peak.h
./Util.h
./C6xSim/c66_ag_intrins.h
./C6xSim/c66_conversion_macros.h
./C6xSim/c66_data_sim.h
./C6xSim/C6xSimulator_base_types.h
./C6xSim/C6xSimulator.h
./C6xSim/C6xSimulator_type_modifiers.h
./C6xSim/_C6xSimulator_priv.h
./C6xSim/global.h
)

add_library(math STATIC ${SOURCES} ${HEADERS})

target_include_directories(math
	PUBLIC ../)


