# TP2 — Navigation (Nav2, ROSMASTER X3)

![distro](https://img.shields.io/badge/ROS2-Jazzy-blue)

## Crédits

Ce TP suit pas à pas le tutoriel [How to Install ROS 2 Navigation (Nav2) – ROS 2 Jazzy](https://automaticaddison.com/how-to-install-ros-2-navigation-nav2-ros-2-jazzy/) d'**Addison Sears-Collins** ([automaticaddison.com](https://automaticaddison.com)), appliqué au robot mecanum ROSMASTER X3 de Yahboom. Il s'appuie sur son repo [`automaticaddison/yahboom_rosmaster`](https://github.com/automaticaddison/yahboom_rosmaster) (branche `jazzy`).

## Objective

Mettre en place la navigation autonome (Nav2) sur un nouveau robot, en Gazebo.

## Setup

```bash
nix develop
colcon build
source install/setup.bash
```

Test rapide — vérifier que le robot se lance et s'affiche dans Gazebo :
```bash
ros2 launch yahboom_rosmaster_gazebo yahboom_rosmaster.gazebo.launch.py
```


