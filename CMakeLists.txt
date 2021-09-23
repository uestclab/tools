cmake_minimum_required(VERSION 3.5)
option(var_platform "This is a default option for ARM64 PLATFORM" ON)
message(STATUS "build public platform switch:"${var_platform})
project(tools)

if(${var_platform} STREQUAL "ON")
	message(STATUS "start to build ARM platform.")
	add_definitions(-DZYNQ)
	# cross complie
	SET(CMAKE_SYSTEM_NAME Linux)
	SET(CMAKE_SYSTEM_PROCESSOR arm) 
	SET(TOOLCHAIN_DIR "/home/dpdk/liqing/petalinux/gcc-arm-linux-gnueabi")
	SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR})
	SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
	SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

	SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-gcc)
	# SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-g++)

elseif(${var_platform} STREQUAL "OFF")
	message(STATUS "start to build ARM64 platform.")
	add_definitions(-DZYNQ_MP)
	SET(CMAKE_SYSTEM_NAME Linux)
	SET(CMAKE_SYSTEM_PROCESSOR arm) 
	SET(TOOLCHAIN_DIR "/home/dpdk/liqing/petalinux/petalinux-2018-2/tools/linux-i386/aarch64-linux-gnu")
	SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR})
	SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
	SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

	SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu-gcc-7.2.1)
	SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu-g++)

endif(${var_platform} STREQUAL "ON")

SET(CMAKE_BUILD_TYPE "Debug")  
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

SET(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin")

SET(CMAKE_VERBOSE_MAKEFILE ON)    
SET(ROOT_PATH ".")

#app
add_subdirectory(${PROJECT_SOURCE_DIR}/app/spi_tool)
#add_subdirectory(${PROJECT_SOURCE_DIR}/app/spi_load_fpga)
# add_subdirectory(${PROJECT_SOURCE_DIR}/app/dac_temp_tool)
add_subdirectory(${PROJECT_SOURCE_DIR}/app/reg_tool)
add_subdirectory(${PROJECT_SOURCE_DIR}/app/rtc_tool)
# add_subdirectory(${PROJECT_SOURCE_DIR}/app/7417_tool)
add_subdirectory(${PROJECT_SOURCE_DIR}/app/gpio_tool)
add_subdirectory(${PROJECT_SOURCE_DIR}/app/rf_7516_tool)
add_subdirectory(${PROJECT_SOURCE_DIR}/app/rf_7516_agc)
# add_subdirectory(${PROJECT_SOURCE_DIR}/app/5682_spi_tool)

# add_subdirectory(${PROJECT_SOURCE_DIR}/app/c++/iqimb)







