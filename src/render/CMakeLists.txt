add_library(
  huli_render INTERFACE
  FPSCounter.hpp
)
add_library(Huli::Render ALIAS huli_render)

target_include_directories(
  huli_render
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_compile_features(huli_render INTERFACE cxx_std_20)
