add_executable(prometheus_util_test
  base64_test.cc
)

target_link_libraries(prometheus_util_test
  PUBLIC
    ${PROJECT_NAME}::util
  PRIVATE
    GTest::gmock_main
)

add_test(
  NAME prometheus_util_test
  COMMAND prometheus_util_test
)
