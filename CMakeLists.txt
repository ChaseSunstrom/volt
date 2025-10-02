cmake_minimum_required(VERSION 3.15)

project(
  volt
  VERSION 1.0
  LANGUAGES C)

set(CMAKE_C_STANDARD 99)
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

# Strict warnings, mapped per-compiler and per-language
target_compile_options(
  ${PROJECT_NAME}
  PRIVATE
    # GCC/Clang (C and C++)
    $<$<AND:$<C_COMPILER_ID:GNU,Clang,AppleClang>>:
    -Wall
    -Wextra
    -Wpedantic
    -Wformat=2
    -Wnull-dereference
    -Wvla
    -Wshadow
    -Wconversion
    -Wno-format-nonliteral # otherwise this will bitch that the fmt.c file is
                           # naughty
    -Wno-int-to-pointer-cast
    -Wno-pointer-to-int-cast
    -Wsign-conversion
    -Wimplicit-fallthrough
    -Werror=return-type
    >
    # Extra C++-only warnings on GCC/Clang
    $<$<AND:$<C_COMPILER_ID:GNU,Clang,AppleClang>,$<COMPILE_LANGUAGE:CXX>>:
    -Wnon-virtual-dtor
    -Woverloaded-virtual
    -Wextra-semi
    >
    # MSVC
    $<$<C_COMPILER_ID:MSVC>:
    /W4
    /permissive-
    /w44265
    /w44062
    /w44242
    /w44194
    /w44165
    /w44996
    >)

# Make warnings fatal (toggle-able)
option(ENABLE_WERROR "Treat warnings as errors" ON)
if(ENABLE_WERROR)
  set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)
endif()

# Enable sanitizers only for GNU/Clang Debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(
    ${PROJECT_NAME}
    PRIVATE
      $<$<AND:$<C_COMPILER_ID:GNU,Clang,AppleClang>>:-fsanitize=address,undefined,leak
      -fno-omit-frame-pointer>)
  target_link_options(
    ${PROJECT_NAME}
    PRIVATE
    $<$<AND:$<C_COMPILER_ID:GNU,Clang,AppleClang>>:-fsanitize=address,undefined,leak>
  )
endif()

option(VOLT_USE_MSVC "Enable VOLT_USE_MSVC" OFF)
target_compile_definitions(${PROJECT_NAME}
                           PRIVATE $<$<BOOL:${VOLT_USE_MSVC}>:VOLT_USE_MSVC=1>)

# MSVC: address sanitizer (VS 2019+), runtime checks
if(MSVC AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(${PROJECT_NAME} PRIVATE /fsanitize=address /RTC1 /sdl)
  target_link_options(${PROJECT_NAME} PRIVATE /fsanitize=address)
endif()

# GNU/Clang hardening (Linux/glibc, adjust per platform)
target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE
    $<$<AND:$<C_COMPILER_ID:GNU,Clang,AppleClang>,$<CONFIG:Release,RelWithDebInfo>>:_FORTIFY_SOURCE=3>
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:Release,RelWithDebInfo>>:_GLIBCXX_ASSERTIONS>
)
target_compile_options(
  ${PROJECT_NAME}
  PRIVATE
    $<$<AND:$<C_COMPILER_ID:GNU,Clang,AppleClang>,$<CONFIG:Release,RelWithDebInfo>>:-fstack-protector-strong>
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${DEPS_DIR}/tomlplusplus/include)

target_precompile_headers(
  ${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/pch.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h)

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
