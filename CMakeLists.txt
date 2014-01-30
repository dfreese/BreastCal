CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)

project (BreastCal)

include(FindROOT.cmake)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

add_subdirectory(src)

include_directories(${ROOT_INCLUDE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)
include_directories(/home/miil/root/macros/include/)

link_directories(${ROOT_LIBRARY_DIR})
link_directories(/home/miil/root/macros/lib)

add_library(ModuleDatDict ModuleDatDict.C 
    src/ModuleDat.C
    src/CoincEvent.C
    src/ModuleCal.C
    src/Apd_Peaktovalley.C)
target_link_libraries(ModuleDatDict myrootlib)
target_link_libraries(ModuleDatDict ${ROOT_LIBRARIES})
target_link_libraries(ModuleDatDict Spectrum)
