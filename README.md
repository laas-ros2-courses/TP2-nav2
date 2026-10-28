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

**5. Configurer la fusion de capteurs (`robot_localization`).**

Créer `yahboom_rosmaster_localization/config/ekf.yaml` :
```bash
cd src/yahboom_rosmaster/yahboom_rosmaster_localization/
mkdir config
touch config/ekf.yaml
```
Y configurer le nœud `ekf_filter_node` du paquet `robot_localization` pour fusionner :
- `/mecanum_drive_controller/odom` (odométrie des roues)
- `/imu/data` (IMU)

Points à régler dans le fichier :
- `frequency`, `two_d_mode: true` (le robot se déplace sur un plan)
- les frames (REP 105) : `map_frame`, `odom_frame`, `base_link_frame: base_footprint`, `world_frame: odom`
- `odom0`/`odom0_config` et `imu0`/`imu0_config` : quels axes de chaque capteur sont fusionnés

Référence officielle pour récupérer un fichier de base : [guide Nav2 "Smoothing Odometry using Robot Localization"](https://docs.nav2.org/jazzy/configuration_and_development/first_time_robot_setup_guide/odom/setup_robot_localization/).

**6. Créer le launch file de l'EKF.**

Créer `yahboom_rosmaster_localization/launch/ekf_gazebo.launch.py` :
```bash
cd src/yahboom_rosmaster/yahboom_rosmaster_localization/
mkdir launch
touch launch/ekf_gazebo.launch.py
```
```python
import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    pkg_share = get_package_share_directory('yahboom_rosmaster_localization')
    ekf_config = os.path.join(pkg_share, 'config', 'ekf.yaml')

    use_sim_time = LaunchConfiguration('use_sim_time')

    declare_use_sim_time = DeclareLaunchArgument(
        name='use_sim_time',
        default_value='true',
        description='Use simulation (Gazebo) clock if true'
    )

    ekf_node = Node(
        package='robot_localization',
        executable='ekf_node',
        name='ekf_filter_node',
        output='screen',
        parameters=[ekf_config, {'use_sim_time': use_sim_time}]
    )

    return LaunchDescription([
        declare_use_sim_time,
        ekf_node
    ])
```

**7. Créer un script combiné (robot + EKF).**

Plutôt que de lancer le robot et l'EKF dans deux terminaux séparés, on peut copier le script existant et y ajouter l'EKF.

Copier `rosmaster_x3_gazebo.sh` :
```bash
cd src/yahboom_rosmaster/yahboom_rosmaster_bringup/scripts/
cp rosmaster_x3_gazebo.sh rosmaster_x3_gazebo_ekf.sh
```

Dans la copie, juste après le bloc d'ajustement de la caméra et avant `wait`, ajouter le lancement de l'EKF en arrière-plan (temps que les contrôleurs, et donc les topics d'odométrie, soient disponibles) :
```bash
echo "Adjusting camera position..."
gz service -s /gui/move_to/pose --reqtype gz.msgs.GUICamera --reptype gz.msgs.Boolean --timeout 2000 --req "pose: {position: {x: 0.0, y: -2.0, z: 2.0} orientation: {x: -0.2706, y: 0.2706, z: 0.6533, w: 0.6533}}"

echo "Launching EKF (robot_localization)..."
ros2 launch yahboom_rosmaster_localization ekf_gazebo.launch.py &

# Keep the script running until Ctrl+C
wait
```

**8. Mettre à jour `CMakeLists.txt`.**

Dans `yahboom_rosmaster_localization/CMakeLists.txt`, installer les dossiers `config` et `launch` :
```cmake
install(
  DIRECTORY config launch
  DESTINATION share/${PROJECT_NAME}
)
```

**9. Builder et lancer.**

```bash
colcon build
source install/setup.bash
```

Avec le script combiné :
```bash
bash src/yahboom_rosmaster/yahboom_rosmaster_bringup/scripts/rosmaster_x3_gazebo_ekf.sh
```

Ou, comme avant, dans deux terminaux séparés.

Terminal 1 :
```bash
bash src/yahboom_rosmaster/yahboom_rosmaster_bringup/scripts/rosmaster_x3_gazebo.sh
```

Terminal 2 :
```bash
ros2 launch yahboom_rosmaster_localization ekf_gazebo.launch.py
```

> ⚠️ Les contrôleurs ROS 2 (`joint_state_broadcaster`, `mecanum_drive_controller`) ne démarrent que **~25 secondes** après le lancement de Gazebo (délai codé dans `load_ros2_controllers.launch.py`). Les topics `/joint_states` et `/mecanum_drive_controller/odom` n'existeront pas avant — c'est normal, pas un bug.

**10. Vérifier la fusion de capteurs.**

```bash
ros2 topic list
ros2 topic info /imu/data -v
ros2 topic echo /imu/data

ros2 topic info /mecanum_drive_controller/odom -v
ros2 topic echo /mecanum_drive_controller/odom

ros2 topic echo /odometry/filtered
```

Vérifier que l'horodatage de ces topics correspond au temps simulation (`ros2 topic echo /clock`).

Déplacer le robot manuellement pour observer la fusion :
```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard --ros-args \
    -p stamped:=true \
    -r /cmd_vel:=/mecanum_drive_controller/cmd_vel
```
> Le `mecanum_drive_controller` attend un `geometry_msgs/msg/TwistStamped` sur `/mecanum_drive_controller/cmd_vel`, pas un `Twist` classique sur `/cmd_vel` — d'où le remapping et le paramètre `stamped:=true`.

Pendant que tu déplaces le robot, comparer en temps réel la vitesse angulaire (yaw) mesurée par chaque capteur et celle calculée par l'EKF — c'est la preuve visuelle que la fusion fonctionne :
```bash
ros2 run rqt_plot rqt_plot
```
> Ajouter les topics un par un dans l'interface (champ **Topic** en haut + bouton **+** vert) plutôt qu'en argument de la commande — `rqt_plot` est peu fiable pour ajouter plusieurs topics directement au lancement.

Ajouter successivement :
- `/mecanum_drive_controller/odom/twist/twist/angular/z`
- `/imu/data/angular_velocity/z`
- `/odometry/filtered/twist/twist/angular/z`

Inspecter la transformée calculée par l'EKF, le nœud, le graphe et l'arbre TF :
```bash
ros2 run tf2_ros tf2_echo odom base_footprint
ros2 node info /ekf_filter_node
rqt_graph
ros2 run tf2_tools view_frames
```


