# find all files constituting this library
file(GLOB imageprocessingfiles
  *.cpp
  external/dgc/*.cpp
  gui/*.cpp
  io/*.cpp
  mser/*.cpp)

# this is a library
add_library(imageprocessing ${imageprocessingfiles})
