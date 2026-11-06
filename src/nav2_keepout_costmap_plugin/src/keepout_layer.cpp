#include "nav2_keepout_costmap_plugin/keepout_layer.hpp"

#include "nav2_costmap_2d/costmap_math.hpp"
#include "pluginlib/class_list_macros.hpp"

using nav2_costmap_2d::LETHAL_OBSTACLE;

namespace nav2_keepout_costmap_plugin
{

void KeepoutLayer::onInitialize()
{
  auto node = node_.lock();

  declareParameter("enabled", rclcpp::ParameterValue(true));
  declareParameter("zone_min_x", rclcpp::ParameterValue(1.0));
  declareParameter("zone_min_y", rclcpp::ParameterValue(1.0));
  declareParameter("zone_max_x", rclcpp::ParameterValue(2.0));
  declareParameter("zone_max_y", rclcpp::ParameterValue(2.0));

  node->get_parameter(name_ + ".enabled", enabled_);
  node->get_parameter(name_ + ".zone_min_x", zone_min_x_);
  node->get_parameter(name_ + ".zone_min_y", zone_min_y_);
  node->get_parameter(name_ + ".zone_max_x", zone_max_x_);
  node->get_parameter(name_ + ".zone_max_y", zone_max_y_);

  current_ = true;
}

void KeepoutLayer::updateBounds(
  double /*robot_x*/, double /*robot_y*/, double /*robot_yaw*/,
  double * min_x, double * min_y, double * max_x, double * max_y)
{
  if (!enabled_) {
    return;
  }

  // La zone est fixe dans le repère "map" : on étend simplement la zone de
  // mise à jour pour qu'elle couvre toujours le rectangle interdit.
  *min_x = std::min(*min_x, zone_min_x_);
  *min_y = std::min(*min_y, zone_min_y_);
  *max_x = std::max(*max_x, zone_max_x_);
  *max_y = std::max(*max_y, zone_max_y_);
}

void KeepoutLayer::updateCosts(
  nav2_costmap_2d::Costmap2D & master_grid,
  int min_i, int min_j, int max_i, int max_j)
{
  if (!enabled_) {
    return;
  }

  for (int j = min_j; j < max_j; j++) {
    for (int i = min_i; i < max_i; i++) {
      double wx, wy;
      master_grid.mapToWorld(i, j, wx, wy);
      if (wx >= zone_min_x_ && wx <= zone_max_x_ &&
        wy >= zone_min_y_ && wy <= zone_max_y_)
      {
        master_grid.setCost(i, j, LETHAL_OBSTACLE);
      }
    }
  }
}

}  // namespace nav2_keepout_costmap_plugin

PLUGINLIB_EXPORT_CLASS(nav2_keepout_costmap_plugin::KeepoutLayer, nav2_costmap_2d::Layer)
