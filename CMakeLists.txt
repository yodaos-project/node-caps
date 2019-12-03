cmake_minimum_required(VERSION 3.6)
project(caps)
set(CMAKE_CXX_STANDARD 11)

file(GLOB SOURCE_FILES "src/*.cc")
file(GLOB JS_FILES *.js *.json)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

if(APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES
                        LINK_FLAGS "-rdynamic -undefined dynamic_lookup")
else()
  set_target_properties(${PROJECT_NAME} PROPERTIES
                        LINK_FLAGS "-rdynamic -Wl,--unresolved-symbols=ignore-all")
endif()

find_library(LIBCAPS libcaps.a)
target_link_libraries(${PROJECT_NAME}
  ${LIBCAPS}
)
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_INCLUDE_DIR}/usr/include/shadow-node
  ${CMAKE_INCLUDE_DIR}/usr/include/caps
)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_DIR})
install(FILES ${JS_FILES} DESTINATION ${CMAKE_INSTALL_DIR})
