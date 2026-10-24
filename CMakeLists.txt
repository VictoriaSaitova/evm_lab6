cmake_minimum_required(VERSION 3.20)
project(evm_lab6 CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(evm_lab6 main.cpp)
target_include_directories(evm_lab6 PRIVATE C:/msys64/ucrt64/include/libusb-1.0)
target_link_libraries(evm_lab6 PRIVATE usb-1.0)