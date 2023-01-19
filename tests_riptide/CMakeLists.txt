

add_executable(synchronous01 synchronous01.cpp)
target_link_libraries(synchronous01 PRIVATE rtac_asio)

add_executable(async01 async01.cpp)
target_link_libraries(async01 PRIVATE rtac_asio)
