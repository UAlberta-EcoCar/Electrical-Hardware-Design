cmake_minimum_required(VERSION 3.20)

project("RF-Module-Firmware-HAL" C CXX ASM)

include(cmake/st-project.cmake)

add_executable(${PROJECT_NAME})
add_st_target_properties(${PROJECT_NAME})

add_subdirectory("rfm95")
target_link_libraries("RF-Module-Firmware-HAL" stm32-hal-rfm95)
include_directories("rfm95")