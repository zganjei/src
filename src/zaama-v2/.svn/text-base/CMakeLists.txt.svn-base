
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

find_package(GMP REQUIRED)
include_directories(${GMP_INCLUDE_DIR})

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

find_path(Z3_INCLUDES NAMES z3++.h )
find_library(Z3_LIBRARIES NAMES z3)

include_directories(${MATHSAT_INCLUDE_DIR})
link_directories(${MATHSAT_LIB_DIR})

include_directories(${Z3_INCLUDES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})


BISON_TARGET(MyParser parser.yy ${CMAKE_CURRENT_BINARY_DIR}/parser.cc)

FLEX_TARGET(MyScanner lexer.ll ${CMAKE_CURRENT_BINARY_DIR}/lexer.cc)

ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

#################################################

#paths for CPLEX:

set(CPLEX_ROOT_DIR "/opt/ibm/ILOG/CPLEX_Studio126/")
include_directories(${CPLEX_ROOT_DIR}/cplex/include/)
include_directories(${CPLEX_ROOT_DIR}/concert/include/)

link_directories(${CPLEX_ROOT_DIR}/cplex/lib/x86-64_linux/static_pic/)
link_directories(${CPLEX_ROOT_DIR}/concert/lib/x86-64_linux/static_pic/)
link_directories(${CPLEX_ROOT_DIR}/opl/lib/x86-64_linux/static_pic/)

#####################################

# enable c++11
#added "-DIL_STD" for CPLEX to compile
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIL_STD -g -ggdb -std=c++11")

#add all libraries to be linked
add_executable(zaama zaama.cpp bw_constraint.cpp linear_expression.cpp ${FLEX_MyScanner_OUTPUTS} ${BISON_MyParser_OUTPUTS} )

target_link_libraries(zaama ${Z3_LIBRARIES})
target_link_libraries(zaama mathsat)
#for cplex
target_link_libraries(zaama ilocplex)
target_link_libraries(zaama cplex)
target_link_libraries(zaama concert)
target_link_libraries(zaama m)
target_link_libraries(zaama pthread)


target_link_libraries(zaama ${GMP_LIBRARIES})

