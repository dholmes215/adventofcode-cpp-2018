add_executable(day01a day01a.cpp)

set_target_properties(day01a
  PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    CXX_STANARD_REQUIRED ON
    CXX_CLANG_TIDY "clang-tidy;-warnings-as-errors=*"
  )

add_executable(day01b day01b.cpp)

set_target_properties(day01b
  PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    CXX_STANARD_REQUIRED ON
    CXX_CLANG_TIDY "clang-tidy;-warnings-as-errors=*"
  )

add_executable(day02a day02a.cpp)

set_target_properties(day02a
  PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    CXX_STANARD_REQUIRED ON
    CXX_CLANG_TIDY "clang-tidy;-warnings-as-errors=*"
  )

add_executable(day02b day02b.cpp)

set_target_properties(day02b
  PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    CXX_STANARD_REQUIRED ON
    CXX_CLANG_TIDY "clang-tidy;-warnings-as-errors=*"
  )

add_executable(day03 day03.cpp)

set_target_properties(day03
  PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    CXX_STANARD_REQUIRED ON
    CXX_CLANG_TIDY "clang-tidy;-warnings-as-errors=*"
  )

add_executable(day04 day04.cpp)

set_target_properties(day04
  PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    CXX_STANARD_REQUIRED ON
# clang-tidy is producing false-positives for this one
#    CXX_CLANG_TIDY "clang-tidy;-warnings-as-errors=*"
  )
