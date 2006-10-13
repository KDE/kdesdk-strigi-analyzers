



########### next target ###############

set(kfile_ts_PART_SRCS kfile_ts.cpp )

kde4_automoc(kfile_ts ${kfile_ts_PART_SRCS})

kde4_add_plugin(kfile_ts ${kfile_ts_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_ts )

target_link_libraries(kfile_ts  ${KDE4_KIO_LIBS} )

install(TARGETS kfile_ts  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_ts.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


