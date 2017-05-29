cmake_minimum_required(VERSION 2.8)
project(aiAccuracy)

set(i3s_source
  i3s/comparedll.cpp
  i3s/compare.cpp
  i3s/fingerprint.cpp
  i3s/element.cpp
  i3s/affine.cpp
  i3s/i3s.cpp
  i3s/compareAllFingers.cpp

  i3s/surf/affine.cpp
  i3s/surf/dllmain.cpp
  i3s/surf/I3SSurf.cpp
  i3s/surf/Line2D.cpp
  i3s/surf/segment.cpp
  i3s/surf/svm.cpp

  )

find_package(Boost COMPONENTS system python thread REQUIRED)
set(PythonLibs_FIND_VERSION 2.7)
find_package(PythonLibs REQUIRED)

set(CMAKE_CXX_FLAGS -std=c++0x ${CMAKE_CXX_FLAGS})

SET(OPENCV_LIBS opencv_nonfree opencv_core opencv_imgproc opencv_ml opencv_highgui opencv_features2d opencv_calib3d)

add_library(i3s SHARED ${i3s_source})
include_directories(${PYTHON_INCLUDE_DIRS})
target_link_libraries(i3s ${Boost_LIBRARIES} ${PYTHON_LIBRARY} ${OPENCV_LIBS})
set_target_properties(i3s PROPERTIES PREFIX "")

set(aphis_source
  aphis/aphis.cpp
  aphis/aphis_impl.cpp)

add_library(aphis SHARED ${aphis_source})
target_link_libraries(aphis ${Boost_LIBRARIES} ${PYTHON_LIBRARY} ${OPENCV_LIBS})
set_target_properties(aphis PROPERTIES PREFIX "")
