cmake_minimum_required(VERSION 3.8)
project(nav2_keepout_costmap_plugin)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav2_costmap_2d REQUIRED)
find_package(pluginlib REQUIRED)

include_directories(include)

add_library(keepout_layer SHARED src/keepout_layer.cpp)
ament_target_dependencies(keepout_layer
  rclcpp
  nav2_costmap_2d
  pluginlib
)

pluginlib_export_plugin_description_file(nav2_costmap_2d keepout_layer.xml)

install(TARGETS keepout_layer
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include/
)

install(FILES keepout_layer.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_export_include_directories(include)
ament_export_libraries(keepout_layer)
ament_export_dependencies(rclcpp nav2_costmap_2d pluginlib)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
