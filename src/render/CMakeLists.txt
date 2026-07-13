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

target_compile_options(
  huli_render
  INTERFACE
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/utf-8>
)

target_compile_definitions(
  huli_render
  INTERFACE
    $<$<PLATFORM_ID:Windows>:NOMINMAX>
)
