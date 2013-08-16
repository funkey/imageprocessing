# find all files constituting this library
file(GLOB imageprocessingfiles
  *.cpp
  mser/*.cpp
  gui/*.cpp
  io/*.cpp)

# this is a library
add_library(imageprocessing ${imageprocessingfiles})
