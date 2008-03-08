include_directories(${STRIGI_INCLUDE_DIR} ${QT_INCLUDES})

# build the analyzer as a module
kde4_add_library(po MODULE poendanalyzer.cpp)

# link with the required libraries
target_link_libraries(po ${STRIGI_STREAMS_LIBRARY} ${STRIGI_STREAMANALYZER_LIBRARY} ${QT_QTCORE_LIBRARY})

# adjust the name so the module is recognized as a strigi plugin
if(WIN32)
  # this is needed to have mingw, cygwin and msvc libs installed in one directory
  if(MSVC)
    set(prefix msvc_strigila_)
  elseif(CYGWIN)
    set(prefix cyg_strigila_)
  elseif(MINGW)
    set(prefix mingw_strigila_)
  endif(MSVC)
else(WIN32)
  set(prefix strigila_)
endif(WIN32)

# install the module in the right directory so it is picked up
install(TARGETS po LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)


install(FILES strigi_translation.fieldproperties
	DESTINATION share/strigi/fieldproperties
)
