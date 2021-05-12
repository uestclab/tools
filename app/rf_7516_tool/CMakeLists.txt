cmake_minimum_required(VERSION 3.5)

include_directories("/home/dpdk/liqing/common/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/app/rf_7516_tool")

SET(BIN_NAME rf_7516_tool) 

#通用源文件 
aux_source_directory(${PROJECT_SOURCE_DIR}/app/rf_7516_tool BIN_SRCS) 

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCS) 
 
add_executable(${BIN_NAME} ${BIN_SRCS} ${SRCS})

add_library(cjson SHARED IMPORTED)

if(${var_platform} STREQUAL "ON")
    # 设置第三方动态库属性
    set_target_properties(cjson PROPERTIES IMPORTED_LOCATION
                        /home/dpdk/liqing/common/lib/libcjson.so)
    install(TARGETS ${BIN_NAME} DESTINATION ${PROJECT_SOURCE_DIR}/bin/arm)
elseif(${var_platform} STREQUAL "OFF")
    # 设置第三方动态库属性
    set_target_properties(cjson PROPERTIES IMPORTED_LOCATION
                        /home/dpdk/liqing/common/lib-arm64/libcjson.so)
    install(TARGETS ${BIN_NAME} DESTINATION ${PROJECT_SOURCE_DIR}/bin/arm64)
endif(${var_platform} STREQUAL "ON")
					  
					  
#link_directories(${PROJECT_SOURCE_DIR}/lib)

target_link_libraries(${BIN_NAME} cjson)



