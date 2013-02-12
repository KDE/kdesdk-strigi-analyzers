project(strigi-analyzer)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

find_package(Strigi REQUIRED)

add_subdirectory( diff )
add_subdirectory( ts )
add_subdirectory( strigi_po )
add_subdirectory( xlf )
