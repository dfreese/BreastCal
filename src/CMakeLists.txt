include_directories(${ROOT_INCLUDE_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)
include_directories(/home/miil/root/macros/include/)

link_directories(/home/miil/root/macros/lib)
link_directories(${ROOT_LIBRARY_DIR})

add_executable(cal_apd_offset cal_apd_offset.C)
target_link_libraries(cal_apd_offset ${ROOT_LIBRARIES})
target_link_libraries(cal_apd_offset Spectrum)
target_link_libraries(cal_apd_offset ModuleDatDict)
target_link_libraries(cal_apd_offset myrootlib)
target_link_libraries(cal_apd_offset Init_avdb)

add_executable(anafloods_psf_v2 anafloods_psf_v2.C FindNext.C Apd_Sort16_v2.C DetCluster.C)
target_link_libraries(anafloods_psf_v2 ${ROOT_LIBRARIES})
target_link_libraries(anafloods_psf_v2 Spectrum)
target_link_libraries(anafloods_psf_v2 ModuleDatDict)
target_link_libraries(anafloods_psf_v2 myrootlib)
target_link_libraries(anafloods_psf_v2 Init_avdb)

add_executable(cal_crystal_offset2 cal_crystal_offset2.C)
target_link_libraries(cal_crystal_offset2 ${ROOT_LIBRARIES})
target_link_libraries(cal_crystal_offset2 ModuleDatDict)
target_link_libraries(cal_crystal_offset2 Init_avdb)

add_executable(cal_edep cal_edep.C)
target_link_libraries(cal_edep ${ROOT_LIBRARIES})
target_link_libraries(cal_edep ModuleDatDict)
target_link_libraries(cal_edep Init_avdb)

add_executable(cal_edep_apd cal_edep_apd.C)
target_link_libraries(cal_edep_apd ${ROOT_LIBRARIES})
target_link_libraries(cal_edep_apd ModuleDatDict)
target_link_libraries(cal_edep_apd Init_avdb)

add_executable(cal_edep_crystal cal_edep_crystal.C)
target_link_libraries(cal_edep_crystal ${ROOT_LIBRARIES})
target_link_libraries(cal_edep_crystal ModuleDatDict)
target_link_libraries(cal_edep_crystal Init_avdb)

add_executable(chain_merged chain_merged.C)
target_link_libraries(chain_merged ${ROOT_LIBRARIES})

add_executable(decoder decoder.C Util.C)
target_link_libraries(decoder ${ROOT_LIBRARIES})
target_link_libraries(decoder ModuleDatDict)
target_link_libraries(decoder Init_avdb)

add_executable(enecal enecal.C)
target_link_libraries(enecal ${ROOT_LIBRARIES})
target_link_libraries(enecal ModuleDatDict)
target_link_libraries(enecal Init_avdb)

add_executable(enefit enefit.C Apd_Fit.C)
target_link_libraries(enefit ${ROOT_LIBRARIES})
target_link_libraries(enefit ModuleDatDict)
target_link_libraries(enefit Init_avdb)
add_executable(fom_ana fom_ana.C)

target_link_libraries(fom_ana ${ROOT_LIBRARIES})
target_link_libraries(fom_ana ModuleDatDict)
target_link_libraries(fom_ana myrootlib)
target_link_libraries(fom_ana Init_avdb)

add_executable(format_recon format_recon.C)
target_link_libraries(format_recon ${ROOT_LIBRARIES})
target_link_libraries(format_recon ModuleDatDict)
target_link_libraries(format_recon Init_avdb)

add_executable(getfloods getfloods.C)
target_link_libraries(getfloods ${ROOT_LIBRARIES})
target_link_libraries(getfloods ModuleDatDict)
#target_link_libraries(getfloods Init_avdb)

add_executable(get_opt_split get_opt_split.C)
target_link_libraries(get_opt_split ${ROOT_LIBRARIES})
target_link_libraries(get_opt_split ModuleDatDict)
target_link_libraries(get_opt_split Init_avdb)

add_executable(merge_4up merge_4up.C)
target_link_libraries(merge_4up ${ROOT_LIBRARIES})
target_link_libraries(merge_4up ModuleDatDict)
target_link_libraries(merge_4up Init_avdb)

add_executable(merge_ana merge_ana.C)
target_link_libraries(merge_ana ${ROOT_LIBRARIES})
target_link_libraries(merge_ana ModuleDatDict)
target_link_libraries(merge_ana Init_avdb)

add_executable(merge_coinc merge_coinc.C)
target_link_libraries(merge_coinc ${ROOT_LIBRARIES})
target_link_libraries(merge_coinc ModuleDatDict)
target_link_libraries(merge_coinc Init_avdb)

add_executable(merge_panel merge_panel.C)
target_link_libraries(merge_panel ${ROOT_LIBRARIES})
target_link_libraries(merge_panel ModuleDatDict)
target_link_libraries(merge_panel Init_avdb)
