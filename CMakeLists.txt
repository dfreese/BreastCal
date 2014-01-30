CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)

project (BreastCal)

include(FindROOT.cmake)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /home/miil/root/macros/lib)

add_subdirectory(src)

MESSAGE( STATUS "ROOT_LIBRARIES:         " ${ROOT_LIBRARIES} )


set(INCLUDE_DIRECTORIES ${ROOT_INCLUDE_DIR})

include_directories(${INCLUDE_DIRECTORIES})
include_directories(${CMAKE_BINARY_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/src)
include_directories(/home/miil/root/macros/include/)

set(LINK_DIRECTORIES ${ROOT_LIBRARY_DIR})
 
link_directories(${LINK_DIRECTORIES})
link_directories(/home/miil/root/macros/lib)


add_library(ModuleDatDict SHARED ModuleDatDict.C 
                                 src/ModuleDat.C
                                 src/CoincEvent.C
                                 src/ModuleCal.C
                                 src/Apd_Peaktovalley.C
                                 /home/miil/root/macros/src/myrootlib)
target_link_libraries(ModuleDatDict ${ROOT_LIBRARIES})
target_link_libraries(ModuleDatDict Spectrum)

#add_executable(fom_ana src/fom_ana.C /home/miil/root/macros/src/myrootlib)
#target_link_libraries(fom_ana ${ROOT_LIBRARIES})
#target_link_libraries(fom_ana ModuleDatDict)
#target_link_libraries(fom_ana libmyrootlib.so)
#target_link_libraries(fom_ana libInit_avdb.so)

