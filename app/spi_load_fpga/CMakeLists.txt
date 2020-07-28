cmake_minimum_required(VERSION 3.5)

include_directories("/home/dpdk/liqing/code/GW_handover/handover_Server/dependence/include/")

include_directories("/home/dpdk/liqing/common/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/app/spi_load_fpga")

SET(BIN_NAME spi_load_fpga) 

#通用源文件 
aux_source_directory(${PROJECT_SOURCE_DIR}/app/spi_tool BIN_SRCS) 

#aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCS) 
 
add_executable(${BIN_NAME} ${BIN_SRCS})

add_library(cjson SHARED IMPORTED)

# 设置第三方动态库属性
set_target_properties(cjson PROPERTIES IMPORTED_LOCATION
                      /home/dpdk/liqing/common/lib/libcjson.so)
					  
					  
#link_directories(${PROJECT_SOURCE_DIR}/lib)

target_link_libraries(${BIN_NAME} cjson)

install(TARGETS ${BIN_NAME} DESTINATION ${PROJECT_SOURCE_DIR}/bin/)
