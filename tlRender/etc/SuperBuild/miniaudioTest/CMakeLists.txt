set(SOURCE main.cpp)
set(HEADER

add_executable(miniaudioTest ${SOURCE} ${HEADER})
add_test(miniaudioTest miniaudioTest)
