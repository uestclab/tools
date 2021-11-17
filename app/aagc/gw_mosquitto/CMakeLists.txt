# CMakeLists for mosquitto client lib

include_directories("/home/dpdk/liqing/code/gw50/dependence/include")

SET(MOSQ_LIB_NAME mosquitto_client)
aux_source_directory(${PROJECT_SOURCE_DIR}/app/aagc/gw_mosquitto MOSQUITTO_SRCS)

add_library(${MOSQ_LIB_NAME} SHARED ${MOSQUITTO_SRCS})

# SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/app/aagc/gw_mosquitto/lib/)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/arm/)

