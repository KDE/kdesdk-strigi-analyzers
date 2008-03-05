include_directories(${STRIGI_INCLUDE_DIR})

# build the analyzer as a module
kde4_add_library(ts MODULE tsthroughanalyzer.cpp)

# link with the required libraries
target_link_libraries(ts ${STRIGI_STREAMANALYZER_LIBRARY} ${KDE4_KIO_LIBS})

# adjust the name so the module is recognized as a strigi plugin
set_target_properties(ts PROPERTIES
    PREFIX strigita_)

# install the module in the right directory so it is picked up
install(TARGETS ts LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)


