find_package(BerkeleyDB REQUIRED)
find_package (Threads REQUIRED)

add_executable(sample-map-usage "sample-map-usage.cpp")
target_link_libraries(sample-map-usage PRIVATE Oracle::BerkeleyDB ${CMAKE_THREAD_LIBS_INIT} stdc++fs)

add_executable(sample-queue-multithread-usage "sample-queue-multithread-usage.cpp")
target_link_libraries(sample-queue-multithread-usage PRIVATE Oracle::BerkeleyDB ${CMAKE_THREAD_LIBS_INIT} stdc++fs)

