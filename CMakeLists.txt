cmake_minimum_required(VERSION 3.15)
project(parsegps C)

add_compile_options(-Wall -Wextra)
add_compile_options(-pedantic)
add_compile_options(-Werror=return-type)
add_compile_options(-Werror=parentheses)
add_compile_options(-Werror=vla)
add_compile_options(-Werror=array-bounds)
add_compile_options ( -Wcast-qual )
add_compile_options ( -Wmaybe-uninitialized )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

set (TARGET ${CMAKE_PROJECT_NAME}.exe)

add_library(libparsegps STATIC 
 "lib/src/UBLOX_NEO_07M.c"
) 

target_include_directories(libparsegps PUBLIC 
"lib/inc"
"lib/src"
)

#target_include_directories(libArray PUBLIC ${PROJECT_SOURCE_DIR}/lib/tests)
#add_library(libArray STATIC lib/tests/catchmain.cpp lib/tests/array2d.cpp)

add_executable(parsegps main.c )
target_link_libraries(parsegps PRIVATE libparsegps)

