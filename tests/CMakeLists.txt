cmake_minimum_required(VERSION 3.20)

project(imtst_testing C CXX)

# Target executables

set(c_target main_c)
set(cpp_target main_cpp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Release")
    set(IMTST_DEBUG OFF)
else()
    set(IMTST_DEBUG ON)
endif()

function(set_test_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
            $<$<CONFIG:Debug>:/W4 /Od /Zi>
            $<$<CONFIG:Release>:/W4 /O2>
        )
    else()
        target_compile_options(${target} PRIVATE
            $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic -O0 -g>
            $<$<CONFIG:Release>:-Wall -Wextra -Wpedantic -O2>
        )
    endif()

    target_compile_definitions(${target} PRIVATE
        $<$<CONFIG:Debug>:IMTST_DEBUG=1>
        $<$<CONFIG:Release>:NDEBUG>
    )
endfunction()

enable_testing()

add_executable(${c_target} main.c)
target_include_directories(${c_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_definitions(${c_target} PRIVATE _POSIX_C_SOURCE=200809L)
set_test_options(${c_target})
add_test(NAME ${c_target} COMMAND ${c_target})

add_executable(${cpp_target} main.cpp)
target_include_directories(${cpp_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
set_test_options(${cpp_target})
add_test(NAME ${cpp_target} COMMAND ${cpp_target})
