



########### next target ###############

set(kfile_diff_PART_SRCS kfile_diff.cpp )

kde4_automoc(kfile_diff ${kfile_diff_PART_SRCS})

kde4_add_plugin(kfile_diff ${kfile_diff_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_diff )

target_link_libraries(kfile_diff  ${KDE4_KIO_LIBS} )

install(TARGETS kfile_diff  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_diff.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


