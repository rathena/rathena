add_executable(prometheus_push_internal_test
  label_encoder_test.cc
)

target_link_libraries(prometheus_push_internal_test
  PRIVATE
    ${PROJECT_NAME}::push_internal_headers
    GTest::gmock_main
)

add_test(
  NAME prometheus_push_internal_test
  COMMAND prometheus_push_internal_test
)
