# 현재 디렉토리에 있는 모든 파일을 SRC_FILES 변수에 추가한다.
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_library(lb_lib STATIC ${SRC_FILES})

# lb_lib 의 include 경로 지정
target_include_directories(lb_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

# lb_lib 의 컴파일 옵션 지정
target_compile_options(lb_lib PRIVATE -Wall -Werror)

# lb_lib 를 C++ 11 로 컴파일
target_compile_features(lb_lib PRIVATE cxx_std_11)


# lb_lib 에 링크
target_link_libraries(lb_lib PRIVATE pthread)

