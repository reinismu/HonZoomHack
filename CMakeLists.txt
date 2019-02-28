cmake_minimum_required(VERSION 3.2)

find_program(
  CLANG_TIDY_EXE
  NAMES "clang-tidy"
  DOC "Path to clang-tidy executable"
  )
if(NOT CLANG_TIDY_EXE)
  message(STATUS "clang-tidy not found.")
else()
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=llvm-*,-clang-analyzer-*,-clang-analyzer-cplusplus*")
endif()

# Have to delete _build so change would take place
set(CMAKE_CXX_COMPILER "clang++")

project(zoomHack)

add_executable(zoomHack zoomhack.cpp Process.cpp Process.h)
set_target_properties(
    zoomHack 
    PROPERTIES 
    CXX_STANDARD 17
)

if(CLANG_TIDY_EXE)
  set_target_properties(
    zoomHack PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
endif()

target_link_libraries(zoomHack "-lstdc++fs")

install(TARGETS zoomHack DESTINATION bin)