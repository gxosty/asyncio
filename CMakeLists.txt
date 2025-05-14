cmake_minimum_required(VERSION 3.18)
set(PROJECT_VERSION 1.0.0)
set(TARGET_NAME "asyncio")
project(${TARGET_NAME}
        VERSION ${PROJECT_VERSION}
        LANGUAGES C CXX)

# --- # --- # --- # --- # --- # --- #

option(WITH_FMT "Build with fmt library for coroutines call stack dumping" OFF)

# set WOLFSSL_INCLUDE_DIR and/or WOLFSSL_LIBRARY_DIR to wolfssl include
# and/or lib directory if it wasn't installed on the system
option(WITH_WOLFSSL "Build with wolfSSL for HTTPS support" OFF)

# --- # --- # --- # --- # --- # --- #

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- # --- # --- # --- # --- # --- #

set(ASYNCIO_INCLUDES
    src/event_loop.cpp
    src/connection.cpp
    src/socket.cpp
)

if(BUILD_SHARED_LIBS)
    add_library(asyncio SHARED ${ASYNCIO_INCLUDES})
else()
    add_library(asyncio STATIC ${ASYNCIO_INCLUDES})
endif(BUILD_SHARED_LIBS)

include(GNUInstallDirs)

if(WITH_WOLFSSL)
    if(WOLFSSL_INCLUDE_DIR)
        target_include_directories(${PROJECT_NAME} PUBLIC ${WOLFSSL_INCLUDE_DIR})
    endif(WOLFSSL_INCLUDE_DIR)
    if(WOLFSSL_LIBRARY_DIR)
        target_link_directories(${PROJECT_NAME} PUBLIC ${WOLFSSL_LIBRARY_DIR})
    endif(WOLFSSL_LIBRARY_DIR)
    target_link_libraries(${PROJECT_NAME} wolfssl)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ASYNCIO_WITH_SSL=1)
endif(WITH_WOLFSSL)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(ASYNCIO_WIN32_SELECTOR_WSAPOLL)
        target_compile_definitions(${PROJECT_NAME} PUBLIC ASYNCIO_WIN32_SELECTOR_WSAPOLL=1)
    else(ASYNCIO_WIN32_SELECTOR_EPOLL)
        include(cmake/wepoll.cmake)
        target_compile_definitions(${PROJECT_NAME} PUBLIC ASYNCIO_WIN32_SELECTOR_EPOLL=1)
    endif()
endif()

set_target_properties(${PROJECT_NAME}
PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
    OUTPUT_NAME ${PROJECT_NAME}
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
)

target_compile_options(${PROJECT_NAME} PUBLIC -fPIC)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/asyncio
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS ${TARGET_NAME} EXPORT ${TARGET_NAME}Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

install(EXPORT ${TARGET_NAME}Targets
    FILE ${TARGET_NAME}Config.cmake
    NAMESPACE ${TARGET_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/asyncio)
