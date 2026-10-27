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

# visualisation du robot qui va etre utilisé
ros2 launch yahboom_rosmaster_gazebo yahboom_rosmaster.gazebo.launch.py 

# Tester que Nav2 est bien installé et fonctionnel
ros2 launch nav2_bringup tb3_simulation_launch.py headless:=False use_composition:=False

# tester le SLAM 
ros2 launch nav2_bringup tb4_simulation_launch.py slam:=True headless:=False use_composition:=False

```

## Steps

**1. Créer les packages `localization` et `navigation`.** Le robot (description, bringup, simulation Gazebo, contrôleur mecanum) est déjà fourni dans `src/yahboom_rosmaster/`. Il manque les deux packages qu'on va écrire pendant ce TP :
```bash
cd src/yahboom_rosmaster/

ros2 pkg create --build-type ament_cmake \
                --license BSD-3-Clause \
                --maintainer-name <ton-nom> \
                --maintainer-email <ton-email> \
                yahboom_rosmaster_localization

ros2 pkg create --build-type ament_cmake \
                --license BSD-3-Clause \
                --maintainer-name <ton-nom> \
                --maintainer-email <ton-email> \
                yahboom_rosmaster_navigation
```
Ajouter ensuite ces deux packages comme `exec_depend` dans `src/yahboom_rosmaster/yahboom_rosmaster/package.xml` (le métapaquet).

**2. Déclarer les dépendances dans les `package.xml`.**

Dans `yahboom_rosmaster_localization/package.xml` :
```xml
<depend>robot_localization</depend>
```

Dans `yahboom_rosmaster_navigation/package.xml` :
```xml
<buildtool_depend>ament_cmake_python</buildtool_depend>

<depend>rclcpp</depend>
<depend>rclpy</depend>
<depend>builtin_interfaces</depend>
<depend>geometry_msgs</depend>
<depend>navigation2</depend>
<depend>nav2_bringup</depend>
<depend>nav2_simple_commander</depend>
<depend>slam_toolbox</depend>
<depend>std_msgs</depend>
<depend>tf2_ros</depend>
<depend>tf_transformations</depend>
```

Déclarer aussi les dépendances correspondantes dans les `CMakeLists.txt` (`find_package`) — uniquement celles utilisées par du code compilé ; les dépendances purement Python ou messages (`builtin_interfaces`, `nav2_simple_commander`, `tf_transformations`) n'ont pas besoin de `find_package`.

Dans `yahboom_rosmaster_localization/CMakeLists.txt` :
```cmake
find_package(robot_localization REQUIRED)
```

Dans `yahboom_rosmaster_navigation/CMakeLists.txt` :
```cmake
find_package(ament_cmake_python REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(navigation2 REQUIRED)
find_package(nav2_bringup REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(slam_toolbox REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
```

**3. Builder le workspace et tester l'installation de Nav2.**

```bash
colcon build
source install/setup.bash
```

**4. Comprendre les transformées (TF).**

Lancer le ROSMASTER X3 dans Gazebo :
```bash
bash src/yahboom_rosmaster/yahboom_rosmaster_bringup/scripts/rosmaster_x3_gazebo.sh
```

Visualiser l'arbre des frames TF, soit en snapshot (PDF) :
```bash
ros2 run tf2_tools view_frames
evince frames_*.pdf
```
soit en temps réel :
```bash
ros2 run rqt_tf_tree rqt_tf_tree
```


