cmake_minimum_required(VERSION 2.8)

# See: https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
SET(CMAKE_CXX_FLAGS "-Ofast")

add_library(MacAuth STATIC
	MacAuth.cpp
	MacAuthUtils.cpp
)

target_link_libraries(MacAuth MacWifiLib gason)

add_application(MacAuthTest
	MacAuthTest.cpp
	MacAuthTest.r
	Keys.cpp
	MacAuth.r
)

target_link_libraries(MacAuthTest MacAuth)

# Add some options to make things smaller.
set_target_properties(MacAuthTest PROPERTIES COMPILE_OPTIONS -ffunction-sections)
set_target_properties(MacAuthTest PROPERTIES LINK_FLAGS "-Wl,-gc-sections")

install(TARGETS MacAuth
    DESTINATION "lib"
)

install(FILES MacAuth.h
    DESTINATION "include/macauth"
)