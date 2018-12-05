add_executable(day01 day01.cpp)

set_target_properties(day01
  PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    CXX_STANARD_REQUIRED ON
    CXX_CLANG_TIDY "clang-tidy;-warnings-as-errors=*"
  )
