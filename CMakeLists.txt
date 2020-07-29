cmake_minimum_required(VERSION 3.5)
option(PUBLIC "This is a default option for PUBLIC" ON)
message(STATUS "build public platform switch:"${PUBLIC})
project(tools)

if(${PUBLIC} STREQUAL "ON")
	message(STATUS "start to build ARM platform.")
	# cross complie
	SET(CMAKE_SYSTEM_NAME Linux)
	SET(CMAKE_SYSTEM_PROCESSOR arm) 
	SET(TOOLCHAIN_DIR "/home/dpdk/liqing/petalinux/gcc-arm-linux-gnueabi")
	SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR})
	SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
	SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

	SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-gcc)
	SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-g++)

elseif(${PUBLIC} STREQUAL "OFF")
    message(STATUS "start to build PC platform.")
	message(STATUS "PROJECT_SOURCE_DIR": ${PROJECT_SOURCE_DIR})
endif(${PUBLIC} STREQUAL "ON")

SET(CMAKE_BUILD_TYPE "Debug")  
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

SET(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin")

SET(CMAKE_VERBOSE_MAKEFILE ON)    
SET(ROOT_PATH ".")

#app
add_subdirectory(${PROJECT_SOURCE_DIR}/app/spi_tool)
#add_subdirectory(${PROJECT_SOURCE_DIR}/app/spi_load_fpga)
add_subdirectory(${PROJECT_SOURCE_DIR}/app/reg_tool)
add_subdirectory(${PROJECT_SOURCE_DIR}/app/rtc_tool)









