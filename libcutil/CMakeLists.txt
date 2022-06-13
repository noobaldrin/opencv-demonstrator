find_package(PkgConfig)
find_package(OpenCV REQUIRED)
pkg_check_modules(GTKMM gtkmm-3.0)

add_library(libcutil src/bytearray.cc
src/cutil.cc
src/erreurs.cc
src/hal.cc
src/journal.cc
src/modele.cc
src/modele-wrapper.cc
src/mxml.cc
src/pugixml.cc)

add_library(mmi src/mmi/ColorCellEditable2.cc
src/mmi/ColorCellRenderer2.cc
src/mmi/docking.cc
src/mmi/field-view.cc
src/mmi/gtkutil.cc
src/mmi/misc.cc
src/mmi/mmi-gen.cc
src/mmi/renderers.cc
src/mmi/serial-ui.cc
src/mmi/stdview.cc
src/mmi/theme.cc)

add_library(comm src/comm/serial-enumeration.cc)

target_include_directories(libcutil PRIVATE
									include/
									include/comm
									${GTKMM_INCLUDE_DIRS}
									)
target_include_directories(mmi PRIVATE include/ include/comm ${GTKMM_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_include_directories(comm PRIVATE include/)

target_link_libraries(mmi PRIVATE ${GTKMM_LIBRARIES})
target_link_libraries(libcutil PRIVATE mmi comm)
