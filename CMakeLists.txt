cmake_minimum_required(VERSION 3.15)

project(
  volt
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
file(MAKE_DIRECTORY ${DEPS_DIR})

set(FETCHCONTENT_GIT_PROTOCOL "https")

message(STATUS "Setting up toml++ git dependency.")
FetchContent_Declare(
  tomlplusplus
  GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
  SOURCE_DIR ${DEPS_DIR}/tomlplusplus)

FetchContent_MakeAvailable(tomlplusplus)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${DEPS_DIR}/tomlplusplus/include)

target_precompile_headers(${PROJECT_NAME} PRIVATE
                          ${CMAKE_CURRENT_SOURCE_DIR}/include/pch.hpp)

# LLVM setup
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# If LLVM doesn’t define the target "LLVM", fall back to components
if(TARGET LLVM)
  target_link_libraries(${PROJECT_NAME} PRIVATE LLVM)
else()
  include_directories(${LLVM_INCLUDE_DIRS})
  add_definitions(${LLVM_DEFINITIONS})
  target_link_directories(${PROJECT_NAME} PRIVATE ${LLVM_LIBRARY_DIRS})
  llvm_map_components_to_libnames(LLVM_LIBS core irreader support)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${LLVM_LIBS})
endif()
