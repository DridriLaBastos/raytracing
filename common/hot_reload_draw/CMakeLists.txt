project(draw)

add_library(hot_reload_draw MODULE hotReloadDraw.cpp)
target_link_libraries(hot_reload_draw sfml-graphics)
