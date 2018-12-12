cmake_minimum_required(VERSION 3.0)
project(caps)
set(CMAKE_CXX_STANDARD 14)
include_directories(node_modules/node-addon-api)
include_directories(node_modules/node-addon-api/src)
include_directories(${CMAKE_JS_INC})
file(GLOB SOURCE_FILES "lib/*.cc" "lib/*.h")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_INCLUDE_DIR}/usr/include/shadow-node
  ${CMAKE_INCLUDE_DIR}/usr/include/caps
)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_DIR})
