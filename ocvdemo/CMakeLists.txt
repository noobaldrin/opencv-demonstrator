file(COPY ${CMAKE_SOURCE_DIR}/ocvdemo/data/
	 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/data )
file(COPY ${CMAKE_SOURCE_DIR}/libcutil/data/std-schema.xml
	 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/data/ )
	 
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

add_executable(ocvdemo src/demos-registration.cc
src/gestion-souris.cc
src/ocvdemo.cc
src/ocvdemo-export.cc
src/ocvdemo-misc.cc
src/ocvdemo-mmi.cc
#src/test-webcam.cc
src/demo-items/demo-fourier.cc
src/demo-items/gradient-demo.cc
src/demo-items/reco-demo.cc
src/demo-items/filtrage.cc
src/demo-items/ocr.cc
src/demo-items/segmentation.cc
src/demo-items/appauto.cc
src/demo-items/3d.cc
src/demo-items/espaces-de-couleurs.cc
src/demo-items/morpho-demo.cc
src/demo-items/misc.cc
src/demo-items/seuillage.cc
src/demo-items/video-demo.cc
src/demo-items/photographie.cc
src/demo-items/histo.cc
src/demo-items/demo-skeleton.cc
src/tools/boutils-image.cc
src/tools/image-mosaic.cc
src/tools/image-selecteur.cc
)

#target_compile_options(ocvdemo PRIVATE -pthread)
target_include_directories(ocvdemo PRIVATE include/ include/demo-items/ ../libcutil/include 
../libocvext/include ${OpenCV_INCLUDE_DIRS} ${GTKMM_INCLUDE_DIRS})
target_link_libraries(ocvdemo PRIVATE libcutil ocvext ${OpenCV_LIBRARIES} ${GTKMM_LIBRARIES} ${Threads_LIBRARIES} 
pthread)
