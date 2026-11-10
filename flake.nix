{
  description = "TP2 INSA : navigation Nav2 (doc officielle générique) sur Yahboom ROSMASTER X3";

  inputs.nix.url = "github:gepetto/nix";

  outputs =
    inputs:
    inputs.nix.lib.mkFlakoboros inputs (
      { lib, ... }:
      {
        rosDistros = [ "jazzy" ];
        rosShellDistro = "jazzy";

        extraPackages = [ "evince" ];

        extraRosPackages = [
          "nav2-bringup"
          "slam-toolbox"
          "rviz2"
          "teleop-twist-keyboard"
          "tf2-tools"
          "rqt-tf-tree"
          "rqt-graph"
          "rqt-plot"

          "navigation2"
          "nav2-simple-commander"
          "robot-localization"
          "tf-transformations"
          "controller-manager"
          "controller-interface"
          "hardware-interface"
          "ros2-control"
          "ros2-controllers"
          "gz-ros2-control"
          "ros-gz"
          "ros-gz-bridge"
          "ros-gz-image"
          "ros-gz-sim"
          "xacro"
          "urdf-tutorial"
          "pal-statistics-msgs"
          "generate-parameter-library"
          "backward-ros"
          "ros2-control-test-assets"
          "ament-lint-auto"
          "ament-lint-common"
          "ament-cmake-gmock"
          "hardware-interface-testing"
        ];
      }
    );
}
