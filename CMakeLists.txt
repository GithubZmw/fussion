cmake_minimum_required(VERSION 3.28)
project(socket)
set(CMAKE_CXX_STANDARD 17)

# 将 CMakeLists.txt 所在目录下的所有 cpp 文件搜索出来并存储到变量 SRC 中
file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/proto/output/*.cc ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)

# 将该工程的源代码编译为可执行文件
add_executable(socket ${SRC})

# 查找系统中的 Protobuf、gflags、OpenSSL 和 LevelDB
find_package(Protobuf REQUIRED)
find_package(gflags REQUIRED)
find_package(OpenSSL REQUIRED)
find_library(GMP gmp REQUIRED)
find_library(GMPXX gmpxx REQUIRED)

# 添加包含路径
target_include_directories(socket PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/libsh
        ${CMAKE_CURRENT_SOURCE_DIR}/libsh/miracl
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/proto
)

# 链接库，包括 OpenSSL 和 LevelDB
target_link_libraries(socket PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/libbrpc.a
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/bls12381.a
        ${GMP}
        ${GMPXX}
        protobuf::libprotobuf
        gflags
        pthread
        OpenSSL::SSL
        OpenSSL::Crypto
        leveldb  # 添加 LevelDB 库
)