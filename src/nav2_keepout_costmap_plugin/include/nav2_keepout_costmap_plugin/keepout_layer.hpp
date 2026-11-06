#ifndef NAV2_KEEPOUT_COSTMAP_PLUGIN__KEEPOUT_LAYER_HPP_
#define NAV2_KEEPOUT_COSTMAP_PLUGIN__KEEPOUT_LAYER_HPP_

#include "rclcpp/rclcpp.hpp"
#include "nav2_costmap_2d/layer.hpp"
#include "nav2_costmap_2d/layered_costmap.hpp"

namespace nav2_keepout_costmap_plugin
{

// Couche de costmap statique : marque un rectangle fixe (repère "map") comme
// infranchissable, pour simuler une zone interdite (ex: zone de stockage,
// zone dangereuse) que la navigation ne doit jamais traverser.
class KeepoutLayer : public nav2_costmap_2d::Layer
{
public:
  KeepoutLayer() = default;

  void onInitialize() override;
  void updateBounds(
    double robot_x, double robot_y, double robot_yaw,
    double * min_x, double * min_y, double * max_x, double * max_y) override;
  void updateCosts(
    nav2_costmap_2d::Costmap2D & master_grid,
    int min_i, int min_j, int max_i, int max_j) override;

  void reset() override {}
  void onFootprintChanged() override {}
  bool isClearable() override {return false;}

private:
  double zone_min_x_{0.0};
  double zone_min_y_{0.0};
  double zone_max_x_{0.0};
  double zone_max_y_{0.0};
};

}  // namespace nav2_keepout_costmap_plugin

#endif  // NAV2_KEEPOUT_COSTMAP_PLUGIN__KEEPOUT_LAYER_HPP_
