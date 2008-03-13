include_directories(${STRIGI_INCLUDE_DIR} ${QT_INCLUDES})

# build the analyzer as a module
kde4_add_library(po MODULE poendanalyzer.cpp)

# link with the required libraries
target_link_libraries(po ${STRIGI_STREAMS_LIBRARY} ${STRIGI_STREAMANALYZER_LIBRARY} ${QT_QTCORE_LIBRARY})

# install the module in the right directory so it is picked up
set_target_properties(po PROPERTIES PREFIX ${STRIGI_LINE_ANALYZER_PREFIX})
install(TARGETS po LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)


install(FILES strigi_translation.fieldproperties
	DESTINATION share/strigi/fieldproperties
)
