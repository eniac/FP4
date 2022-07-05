cmake_minimum_required(VERSION 3.0.2)

project(Instrumentation)

message("${CMAKE_SYSTEM_NAME} system")

if(VERBOSE)
    add_definitions(-DVERBOSE)
    message("VERBOSE")
endif()
if(INFO)
    add_definitions(-DINFO)
    message("INFO")
endif()

# *.c compiled as .cpp
file(GLOB_RECURSE CFILES "${CMAKE_SOURCE_DIR}/*.c")
SET_SOURCE_FILES_PROPERTIES(${CFILES} PROPERTIES LANGUAGE CXX)

set(CMAKE_CXX_STANDARD 11)
message("-std=c++: ${CMAKE_CXX_STANDARD}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format -Wno-deprecated-register -g -rdynamic -std=c++11")

# For easy usage of shell scripts
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})

find_package(BISON)
find_package(FLEX)

if(BISON_FOUND)
    message("Bison found")
elseif(NOT BISON_FOUND)
    error("Bison Not Found")
endif()
if(FLEX_FOUND)
    message("Flex found")
elseif(NOT FLEX_FOUND)
    error("Flex Not Found")
endif()

set(MYSOURCES
    util/util.cpp
)

BISON_TARGET(MyParser frontend.y ${CMAKE_CURRENT_BINARY_DIR}/frontend.tab.c
                      DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/frontend.tab.h)
FLEX_TARGET(MyScanner frontend.l  ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

file(GLOB MYINCLUDES include/*)

foreach(file ${BISON_MyParser_OUTPUTS})
    message("Bison Filename: ${file}")
endforeach()

SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/frontend.tab.c PROPERTIES LANGUAGE CXX)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c PROPERTIES LANGUAGE CXX)

# Or CMAKE_SOURCE_DIR, so that flex/bison generated file could find the path
include_directories(${CMAKE_HOME_DIRECTORY})

add_library(ast SHARED src/ast/ast_nodes_p4.cpp)
add_library(parser SHARED src/modifier/parser_graph.cpp)
add_library(static_analysis SHARED src/static_analysis/static_analysis.cpp)
add_library(modifier SHARED src/modifier/modifier.cpp)
add_library(modifier_ut SHARED src/modifier/ut.cpp)
add_library(modifier_dt SHARED src/modifier/dt.cpp)

add_executable(frontend
    ${MYSOURCES}
    ${MYINCLUDES}
    ${BISON_MyParser_OUTPUTS}
    ${FLEX_MyScanner_OUTPUTS}
)
target_link_libraries(frontend ast)
target_link_libraries(frontend parser)
target_link_libraries(frontend static_analysis)
target_link_libraries(frontend modifier)
target_link_libraries(frontend modifier_ut)
target_link_libraries(frontend modifier_dt)
