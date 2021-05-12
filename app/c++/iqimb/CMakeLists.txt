cmake_minimum_required(VERSION 3.5)
option(PLATFORM "This is a default option for Arm PLATFORM" ON)
message(STATUS "build public platform switch:"${PLATFORM})

SET(BIN_NAME iq_imbalance) 

include_directories("/home/dpdk/liqing/code/cross_compile/eigen-3.2.10/_install/include/eigen3")

if(${PLATFORM} STREQUAL "ON")
	message(STATUS "start to build ARM platform.")
	# zlog
	link_directories("/home/dpdk/liqing/common/lib/")
	include_directories("/home/dpdk/liqing/common/include/")

	SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-gcc-6.2.1)
	SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-g++)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O2 -std=c++11 -mfpu=neon -mcpu=cortex-a9")

elseif(${PLATFORM} STREQUAL "OFF")
    message(STATUS "iq imbalance start to build PC platform.")
	message(STATUS "PROJECT_SOURCE_DIR": ${PROJECT_SOURCE_DIR})

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O2 -std=c++11 ")

endif(${PLATFORM} STREQUAL "ON")

# SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")  
# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

SET(CMAKE_VERBOSE_MAKEFILE ON)    
SET(ROOT_PATH ".")

#通用源文件 
aux_source_directory(${PROJECT_SOURCE_DIR}/app/c++/iqimb BIN_SRCS) 
 
add_executable(${BIN_NAME} ${BIN_SRCS})

if(${PLATFORM} STREQUAL "ON")
	add_library(zlog SHARED IMPORTED)
	# 设置第三方动态库属性
	set_target_properties(zlog PROPERTIES IMPORTED_LOCATION
						/home/dpdk/liqing/common/lib/libzlog.so) 
	target_link_libraries(${BIN_NAME} zlog)
elseif(${PLATFORM} STREQUAL "OFF")

endif(${PLATFORM} STREQUAL "ON")

install(TARGETS ${BIN_NAME} DESTINATION ${PROJECT_SOURCE_DIR}/bin/c++/)










