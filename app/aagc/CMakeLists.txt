cmake_minimum_required(VERSION 3.5)

include_directories("/home/dpdk/liqing/common/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/app/aagc")

SET(BIN_NAME aagc) 

#通用源文件 
aux_source_directory(${PROJECT_SOURCE_DIR}/app/aagc BIN_SRCS) 

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCS) 
 
add_executable(${BIN_NAME} ${BIN_SRCS} ${SRCS})

add_library(cjson SHARED IMPORTED)
add_library(zlog SHARED IMPORTED)

if(${var_platform} STREQUAL "ON")
    set_target_properties(cjson PROPERTIES IMPORTED_LOCATION
                        /home/dpdk/liqing/common/lib/libcjson.so)
    set_target_properties(zlog PROPERTIES IMPORTED_LOCATION 
                        /home/dpdk/liqing/common/lib/libzlog.so)
    install(TARGETS ${BIN_NAME} DESTINATION ${PROJECT_SOURCE_DIR}/bin/arm)
elseif(${var_platform} STREQUAL "OFF")
    set_target_properties(cjson PROPERTIES IMPORTED_LOCATION
                        /home/dpdk/liqing/common/lib-arm64/libcjson.so)
    set_target_properties(zlog PROPERTIES IMPORTED_LOCATION 
                        /home/dpdk/liqing/common/lib-arm64/libzlog.so)
    install(TARGETS ${BIN_NAME} DESTINATION ${PROJECT_SOURCE_DIR}/bin/arm64)
endif(${var_platform} STREQUAL "ON")

#link_directories(${PROJECT_SOURCE_DIR}/lib)

target_link_libraries(${BIN_NAME} cjson)
target_link_libraries(${BIN_NAME} zlog)
target_link_libraries(${BIN_NAME} -lm)


