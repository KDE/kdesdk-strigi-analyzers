

########### next target ###############

set(kfile_cpp_PART_SRCS kfile_cpp.cpp )

kde4_automoc(${kfile_cpp_PART_SRCS})

kde4_add_plugin(kfile_cpp ${kfile_cpp_PART_SRCS})



target_link_libraries(kfile_cpp ${KDE4_KIO_LIBS} )

install(TARGETS kfile_cpp  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_h.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

