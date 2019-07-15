find_package(BerkeleyDB REQUIRED)
find_package (Threads REQUIRED)

SET(SAMPLES_LIBS Oracle::BerkeleyDB ${CMAKE_THREAD_LIBS_INIT} stdc++fs)

add_executable(sample-map-usage "sample-map-usage.cpp")
target_link_libraries(sample-map-usage PRIVATE ${SAMPLES_LIBS})
target_compile_definitions(sample-map-usage PRIVATE ENV_FOLDER="sample-map-usage-dir")

add_executable(sample-queue-multithread-usage "sample-queue-multithread-usage.cpp")
target_link_libraries(sample-queue-multithread-usage PRIVATE ${SAMPLES_LIBS})
target_compile_definitions(sample-queue-multithread-usage PRIVATE ENV_FOLDER="sample-queue-multithread-usage-dir")

