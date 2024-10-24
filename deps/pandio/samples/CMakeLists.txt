cmake_minimum_required(VERSION 3.22)
project(pandio_samples C)

add_subdirectory(.. pandio_build)

add_executable(sample_server server.c)
target_link_libraries(sample_server PRIVATE pandio)

add_executable(sample_client client.c)
target_link_libraries(sample_client PRIVATE pandio)

add_executable(sample_random random.c)
target_link_libraries(sample_random PRIVATE pandio)
