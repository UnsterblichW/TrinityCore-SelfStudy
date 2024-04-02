
find_package(Boost 1.78 REQUIRED system)

add_executable(server main.cpp EchoSession.cpp)

target_include_directories(server
  PUBLIC
    ${Boost_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Networking)

target_link_libraries(server
  PUBLIC
    Boost::system
    pthread)
