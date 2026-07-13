add_executable(
  huli_example1
  main.cpp
)

target_compile_features(
  huli_example1
  PRIVATE cxx_std_20
)

target_link_libraries(
  huli_example1
  PRIVATE
    Huli::Render
    Huli::Vulkan
    glfw
)

target_compile_definitions(
  huli_example1
  PRIVATE
    HULI_EXAMPLE1_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)
