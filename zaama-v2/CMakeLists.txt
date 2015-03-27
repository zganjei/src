
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

find_package(GMP REQUIRED)
include_directories(${GMP_INCLUDE_DIR})

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

find_path(Z3_INCLUDES NAMES z3++.h )
find_library(Z3_LIBRARIES NAMES z3)

message (${Z3_INCLUDES})
message (${Z3_LIBRARIES})

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

set(CPLEX_INSTALLATION "/opt/ibm/ILOG/CPLEX_Studio126/")
include_directories(${CPLEX_INSTALLATION}/cplex/include/)
include_directories(${CPLEX_INSTALLATION}/concert/include/)

link_directories(/opt/ibm/ILOG/CPLEX_Studio126/cplex/lib/x86-64_linux/static_pic/)
link_directories(/opt/ibm/ILOG/CPLEX_Studio126/concert/lib/x86-64_linux/static_pic/)
link_directories(/opt/ibm/ILOG/CPLEX_Studio126/opl/lib/x86-64_linux/static_pic/)
#####################################

# enable c++11
#zeinab: I added "-DIL_STD" for CPLEX to compile
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIL_STD -g -ggdb -std=c++11")

#
#inja bayad hame library ha ro ezafe konim ta link beshan! :)
add_executable(zaama_new zaama.cpp bw_constraint.cpp linear_expression.cpp ${FLEX_MyScanner_OUTPUTS} ${BISON_MyParser_OUTPUTS} )

target_link_libraries(zaama_new ${Z3_LIBRARIES})
target_link_libraries(zaama_new mathsat)
#for cplex
target_link_libraries(zaama_new ilocplex)
target_link_libraries(zaama_new cplex)
target_link_libraries(zaama_new concert)
target_link_libraries(zaama_new m)
target_link_libraries(zaama_new pthread)


target_link_libraries(zaama_new ${GMP_LIBRARIES})

