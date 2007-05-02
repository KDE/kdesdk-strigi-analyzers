include_directories(${STRIGI_INCLUDE_DIR})

# build the analyzer as a module
kde4_add_library(diff MODULE difflineanalyzer.cpp)

# link with the required libraries
target_link_libraries(diff ${STREAMANALYZER_LIBRARY} ${KDE4_KIO_LIBS})

# adjust the name so the module is recognized as a strigi plugin
set_target_properties(diff PROPERTIES
    PREFIX strigila_)

# install the module in the right directory so it is picked up
install(TARGETS diff LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)

