# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\RPGByAI_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\RPGByAI_autogen.dir\\ParseCache.txt"
  "RPGByAI_autogen"
  )
endif()
