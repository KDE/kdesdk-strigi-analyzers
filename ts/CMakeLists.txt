include_directories(${STRIGI_INCLUDE_DIR})

# build the analyzer as a module
kde4_add_library(ts MODULE tsthroughanalyzer.cpp)

# link with the required libraries
target_link_libraries(ts ${STRIGI_STREAMANALYZER_LIBRARY} ${KDE4_KIO_LIBS})

# adjust the name so the module is recognized as a strigi plugin
if(WIN32)
  # this is needed to have mingw, cygwin and msvc libs installed in one directory
  if(MSVC)
    set(prefix msvc_strigita_)
  elseif(CYGWIN)
    set(prefix cyg_strigita_)
  elseif(MINGW)
    set(prefix mingw_strigita_)
  endif(MSVC)
else(WIN32)
  set(prefix strigita_)
endif(WIN32)

# install the module in the right directory so it is picked up
install(TARGETS ts LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)


