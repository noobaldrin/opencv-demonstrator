find_package(OpenCV REQUIRED)

add_library(ocvext src/calib.cc
src/fourier.cc
src/gl.cc
src/hough.cc
src/images-selection.cc
src/ocvext.cc
src/reco.cc
src/thinning.cc
src/vue-collection.cc
src/vue-image.cc)

target_include_directories(ocvext PRIVATE
								include/ 
								../libcutil/include
								${OpenCV_INCLUDE_DIRS}
								${GTKMM_INCLUDE_DIRS})
target_link_libraries(ocvext PRIVATE ${OpenCV_LIBRARIES} ${GTKMM_LIBRARIES})
