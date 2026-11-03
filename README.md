# TP2 — Navigation (Nav2, ROSMASTER X3)

![distro](https://img.shields.io/badge/ROS2-Jazzy-blue)

## Crédits

Ce TP utilise les paquets du robot d'**Addison Sears-Collins** utilisé dans son tutoriel [How to Install ROS 2 Navigation (Nav2) – ROS 2 Jazzy](https://automaticaddison.com/how-to-install-ros-2-navigation-nav2-ros-2-jazzy/) ([automaticaddison.com](https://automaticaddison.com)). Il s'appuie sur son repo [`automaticaddison/yahboom_rosmaster`](https://github.com/automaticaddison/yahboom_rosmaster) (branche `jazzy`).

## Objective

Mettre en place la navigation autonome (Nav2) sur un nouveau robot, en Gazebo.

## Setup

**1. Installer l'environnement et vérifier que tout fonctionne.**

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

**2. Créer les packages `localization` et `navigation`.**

Le robot (description, bringup, simulation Gazebo, contrôleur mecanum) est déjà fourni dans `src/yahboom_rosmaster/` :

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

**3. Déclarer les dépendances dans les `package.xml`.**

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

**4. Déclarer les dépendances dans les `CMakeLists.txt` (`find_package`).**

Uniquement celles utilisées par du code compilé ; les dépendances purement Python ou messages (`builtin_interfaces`, `nav2_simple_commander`, `tf_transformations`) n'ont pas besoin de `find_package`.

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

**5. Build.**

```bash
colcon build
source install/setup.bash
```

## Comprendre les transformées (TF)

**1. Lancer le ROSMASTER X3 dans Gazebo.**
```bash
bash src/yahboom_rosmaster/yahboom_rosmaster_bringup/scripts/rosmaster_x3_gazebo.sh
```

**2. Visualiser l'arbre des frames TF.**

Soit en snapshot (PDF) :
```bash
ros2 run tf2_tools view_frames
evince frames_*.pdf
```
soit en temps réel :
```bash
ros2 run rqt_tf_tree rqt_tf_tree
```

## Localisation et fusion de capteurs (robot_localization)

**1. Créer `yahboom_rosmaster_localization/config/ekf.yaml`.**
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

**2. Le launch file de l'EKF.**

Le nœud `ekf_filter_node` est lancé par un launch file minimal — déjà fourni dans `yahboom_rosmaster_bringup/launch/ekf_gazebo.launch.py` (branche `main`). Ouvir le fichier afin de prendre en main son contenu. Comme pour tous les fichiers qui combinent/orchestrent plusieurs morceaux dans ce TP, il vit dans `yahboom_rosmaster_bringup`, pas dans le paquet qu'on écrit.


**3. Le launch file et le script combinés (robot + EKF).**

Toujours dans `yahboom_rosmaster_bringup`, déjà fournis — `launch/rosmaster_x3_localization.launch.py` et `scripts/rosmaster_x3_localization.sh` :
- Le launch file inclut (`IncludeLaunchDescription`) le launch de Gazebo et celui de l'EKF dans une seule `LaunchDescription`. L'EKF attend juste que `/clock` et les topics d'odométrie/IMU existent.

**4. Mettre à jour `CMakeLists.txt`.**

Dans `yahboom_rosmaster_localization/CMakeLists.txt`, installer le dossier `config` :
```cmake
install(
  DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)
```

**5. Build et lancer.**

```bash
colcon build
source install/setup.bash
```

Avec le script combiné :
```bash
bash src/yahboom_rosmaster/yahboom_rosmaster_bringup/scripts/rosmaster_x3_localization.sh
```

> ⚠️ Les contrôleurs ROS 2 (`joint_state_broadcaster`, `mecanum_drive_controller`) ne démarrent que **~25 secondes** après le lancement de Gazebo (délai codé dans `load_ros2_controllers.launch.py`). Les topics `/joint_states` et `/mecanum_drive_controller/odom` n'existeront pas avant — c'est normal, pas un bug.

**6. Vérifier la fusion de capteurs.**

```bash
ros2 topic list
ros2 topic info /imu/data -v
ros2 topic echo /imu/data

ros2 topic info /mecanum_drive_controller/odom -v
ros2 topic echo /mecanum_drive_controller/odom

ros2 topic echo /odometry/filtered
```

Vérifier que l'horodatage de ces topics correspond au temps simulation (`ros2 topic echo /clock`).

**7. Déplacer le robot manuellement pour observer la fusion.**
```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard --ros-args \
    -p stamped:=true \
    -r /cmd_vel:=/mecanum_drive_controller/cmd_vel
```
> Le `mecanum_drive_controller` attend un `geometry_msgs/msg/TwistStamped` sur `/mecanum_drive_controller/cmd_vel`, pas un `Twist` classique sur `/cmd_vel` — d'où le remapping et le paramètre `stamped:=true`.

**8. Comparer en temps réel les vitesses mesurées et estimées.**

Pendant que le robot bouge, comparer la vitesse angulaire (yaw) mesurée par chaque capteur et celle calculée par l'EKF — c'est la preuve visuelle que la fusion fonctionne :
```bash
ros2 run rqt_plot rqt_plot
```
> Ajouter les topics un par un dans l'interface (champ **Topic** en haut + bouton **+** vert) plutôt qu'en argument de la commande — `rqt_plot` est peu fiable pour ajouter plusieurs topics directement au lancement.

Ajouter successivement :
- `/mecanum_drive_controller/odom/twist/twist/angular/z`
- `/imu/data/angular_velocity/z`
- `/odometry/filtered/twist/twist/angular/z`

**9. Inspecter la transformée, le nœud, le graphe et l'arbre TF.**
```bash
ros2 run tf2_ros tf2_echo odom base_footprint
ros2 node info /ekf_filter_node
rqt_graph
ros2 run tf2_tools view_frames
```

## Explorer les capteurs disponibles

**1. Lancer le robot (si ce n'est pas déjà fait).**
```bash
bash src/yahboom_rosmaster/yahboom_rosmaster_bringup/scripts/rosmaster_x3_localization.sh
```

**2. Ajouter les affichages capteurs dans RViz.**
- Champ **Fixed Frame** (panneau Displays) : mettre `odom`.
- **Add** → onglet **By topic** → sélectionner `/scan` (`sensor_msgs/LaserScan`). Dans les propriétés de l'affichage, mettre la **Reliability Policy** sur `Best Effort` (sinon RViz n'affiche rien si des paquets du LIDAR sont perdus/retardés), et la **Size** sur `0.1` pour mieux voir les points.
- **Add** → onglet **By topic** → sélectionner `/imu/data` (`sensor_msgs/Imu`).

**3. Déplacer le robot et observer.**

Avec le téléop (section précédente), observer en temps réel : le scan laser qui correspond aux obstacles dans Gazebo, et l'orientation/accélération de l'IMU qui bouge avec le robot.

## Construire une carte de l'environnement en utilisant le SLAM

**1. Créer les dossiers `config` et `maps`.**

```bash
cd src/yahboom_rosmaster/yahboom_rosmaster_navigation/
mkdir config maps
```

**2. Créer le fichier de configuration Nav2 (`nav2_params.yaml`).**

```bash
cd config
touch nav2_params.yaml
```

Copier le contenu du fichier par défaut officiel de Nav2 : [`nav2_params.yaml`](https://github.com/ros-navigation/navigation2/blob/jazzy/nav2_bringup/params/nav2_params.yaml) (branche `jazzy`).

Ce fichier est complété par étapes : seul le bloc `slam_toolbox` est configuré ici. Les blocs `amcl`, `global_costmap`/`local_costmap`, `bt_navigator`, `controller_server` et `velocity_smoother` sont expliqués dans la section *Navigation autonome* — ils tournent déjà (`nav2_bringup` les démarre dans tous les cas, SLAM ou pas), mais avec leurs valeurs par défaut (pensées pour un robot différentiel) tant qu'on ne les a pas adaptés ; ça n'empêche pas le SLAM de fonctionner, puisqu'on ne fait ici que du téléop manuel.

**3. Configurer `slam_toolbox`.**

Le paquet `slam_toolbox` permet au robot de construire une carte de son environnement pendant qu'il se déplace (SLAM = *Simultaneous Localization And Mapping*). En explorant, le robot détecte les obstacles avec son LIDAR, se localise par rapport à eux, et ajoute ces informations à la carte en cours de construction. `slam_toolbox` est aussi responsable de publier la transformée entre les frames `map` et `odom` (chez nous, cette `odom` est déjà corrigée par l'EKF — section *Localisation et fusion de capteurs*).

Ajouter un bloc `slam_toolbox:` dans `nav2_params.yaml`, à partir du fichier par défaut du paquet (mode *online async*, le plus courant pour une utilisation en direct pendant que le robot se déplace) : [`mapper_params_online_async.yaml`](https://github.com/SteveMacenski/slam_toolbox/blob/jazzy/config/mapper_params_online_async.yaml).

Contrairement à `amcl`, rien à adapter ici : `base_frame: base_footprint` et `scan_topic: /scan` correspondent déjà à ce qu'on a mis en place.

**4. Supprimer les blocs inutilisés.**

Le fichier de base contient aussi `docking_server`, `route_server` et `loopback_simulator`, trois éléments non utilisés dans ce TP :
- `docking_server` : gère un dock de charge automatique — le X3 n'en a pas ici.
- `route_server` : navigation sur un graphe de routes prédéfini (typiquement pour un entrepôt avec des allées fixes) — on navigue en espace libre, pas sur un graphe.
- `loopback_simulator` : un faux simulateur (sans physique) qui permet de tester Nav2 sans Gazebo ni robot réel — on a déjà Gazebo comme simulateur.

Supprimer les blocs `route_server` et `loopback_simulator` de `nav2_params.yaml`.

`docking_server`, lui, ne peut pas être entièrement supprimé : `nav2_bringup` le démarre quoi qu'il arrive — en mode SLAM comme en mode navigation —, et refuse de démarrer tout le reste si ce nœud échoue à se configurer. Sans aucun `dock_plugins` déclaré, il échoue avec `Charging dock plugins not given!` et bloque tout Nav2 — et une liste vide (`dock_plugins: []`) ne marche pas non plus : `rclcpp` ne peut pas déduire le type d'un tableau YAML vide, et le nœud plante (`SIGABRT`) au lieu d'échouer proprement, ce qui bloque `lifecycle_manager_navigation` indéfiniment. Il faut donc déclarer un vrai plugin, même inutilisé dans ce TP (pas de dock physique/simulé) — **sans ce fix, même le lancement en mode SLAM pur, à l'étape suivante, reste planté** :
```yaml
docking_server:
  ros__parameters:
    dock_plugins: ["simple_charging_dock"]
    simple_charging_dock:
      plugin: "opennav_docking::SimpleChargingDock"
```

**5. Créer le nœud relais `cmd_vel` → `TwistStamped`.**

Nav2 (`controller_server`, `velocity_smoother`) publie les commandes de vitesse en `geometry_msgs/Twist` classique sur `/cmd_vel`, sans horodatage. Le `mecanum_drive_controller` attend un `geometry_msgs/TwistStamped` (même contrainte que pour le téléop, section *Localisation et fusion de capteurs*) — mais contrairement à `teleop_twist_keyboard`, Nav2 n'a pas de paramètre `stamped` intégré. Il faut donc un petit nœud qui convertit l'un vers l'autre.

Ce nœud ne sert vraiment qu'en navigation autonome (tant qu'on reste en SLAM pur avec le téléop, personne ne lui envoie rien) — mais `rosmaster_x3_navigation.launch.py` inclut ce nœud **sans condition**, qu'on soit en mode SLAM ou navigation. Il doit donc déjà exister avant même le premier lancement ci-dessous, sinon `ros2 launch` échoue sur ce nœud (`libexec directory ... does not exist`) — même logique que pour le fix `docking_server` de l'étape précédente.

Créer le fichier :
```bash
cd src/yahboom_rosmaster/yahboom_rosmaster_navigation/
mkdir scripts
touch scripts/cmd_vel_relay.py
chmod +x scripts/cmd_vel_relay.py
```
```python
#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist, TwistStamped


class CmdVelRelay(Node):
    def __init__(self):
        super().__init__('cmd_vel_relay')
        self.pub = self.create_publisher(TwistStamped, '/mecanum_drive_controller/cmd_vel', 10)
        self.sub = self.create_subscription(Twist, 'cmd_vel', self.callback, 10)

    def callback(self, msg: Twist):
        out = TwistStamped()
        out.header.stamp = self.get_clock().now().to_msg()
        out.twist = msg
        self.pub.publish(out)


def main():
    rclpy.init()
    rclpy.spin(CmdVelRelay())


if __name__ == '__main__':
    main()
```

**6. Installer le script et les dossiers de config.**

```cmake
install(PROGRAMS
  scripts/cmd_vel_relay.py
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY config maps
  DESTINATION share/${PROJECT_NAME}
)
```
Ce deuxième bloc (même principe que pour `yahboom_rosmaster_localization`, section *Localisation et fusion de capteurs*) est nécessaire pour que `nav2_params.yaml` et les cartes sauvegardées (étape suivante) soient accessibles une fois le paquet installé.

**7. Builder et tester le relais.**

```bash
colcon build
source install/setup.bash
ros2 run yahboom_rosmaster_navigation cmd_vel_relay.py
```

Dans un autre terminal, publier un `Twist` de test sur `/cmd_vel` et vérifier qu'il ressort bien en `TwistStamped` sur `/mecanum_drive_controller/cmd_vel` :
```bash
ros2 topic pub /cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.2}}" --once
ros2 topic echo /mecanum_drive_controller/cmd_vel
```

**8. Lancer le SLAM et explorer l'environnement.**

Avec `slam_toolbox` configuré, les blocs problématiques nettoyés et le relais en place, lancer la simulation en mode SLAM — c'est le comportement par défaut du script, pas besoin d'argument :

```bash
colcon build
source install/setup.bash
bash src/yahboom_rosmaster/yahboom_rosmaster_bringup/scripts/rosmaster_x3_navigation.sh
```

Dans RViz, ajouter un affichage **By topic** → `/map` (`nav_msgs/OccupancyGrid`) : la carte se construit en direct au fur et à mesure que le robot explore.

Déplacer le robot au clavier pour parcourir tout l'environnement et couvrir la carte avec le LIDAR (même commande qu'à la section *Localisation et fusion de capteurs*) :
```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard --ros-args \
    -p stamped:=true \
    -r /cmd_vel:=/mecanum_drive_controller/cmd_vel
```

**9. Sauvegarder la carte.**

Une fois l'environnement bien exploré, sauvegarder la carte construite par `slam_toolbox` :
```bash
ros2 run nav2_map_server map_saver_cli -f src/yahboom_rosmaster/yahboom_rosmaster_navigation/maps/cafe_world_map --ros-args -p save_map_timeout:=10.0
```
> Le timeout par défaut de `map_saver_cli` (2 s) est souvent trop court et fait échouer la sauvegarde (`Failed to spin map subscription`) même quand `/map` est bien publié — d'où le `save_map_timeout:=10.0`.

Ça produit `cafe_world_map.yaml` (métadonnées) et `cafe_world_map.pgm` (l'image de la carte) dans `maps/` — exactement le fichier que `rosmaster_x3_navigation.launch.py` charge par défaut (argument `map`) quand on lance en mode `localize` (section *Navigation autonome*).

> ⚠️ Comme pour `nav2_params.yaml`, ce dossier `maps/` n'est copié dans `install/` (d'où le launch file charge la carte) qu'au prochain `colcon build` — sans ça, lancer en mode `localize` juste après avoir sauvegardé échoue (`Failed to load map yaml file`). Les builds des étapes suivantes s'en chargent naturellement, mais si tu testes `localize` directement après avoir sauvegardé la carte, pense à rebuilder avant.

## Navigation autonome

**1. Configurer `amcl`.**

Le nœud `amcl` (*Adaptive Monte Carlo Localization*) sert à localiser le robot sur une carte déjà construite : il compare les scans LIDAR à la carte connue pour estimer la position et l'orientation du robot, via un filtre à particules (chaque particule est une hypothèse de pose du robot). Contrairement à `slam_toolbox` (section précédente), `amcl` ne tourne que lorsqu'on navigue sur une carte déjà enregistrée (mode `localize`) — pas en mode SLAM.

Le fichier de base est prévu pour un robot à entraînement différentiel. Le ROSMASTER X3 est un robot **mecanum** (il peut se déplacer latéralement, pas seulement avancer/tourner) : il faut donc adapter le modèle de mouvement utilisé par `amcl`.

Sur la page de configuration `amcl` de la doc Nav2 ([lien](https://docs.nav2.org/jazzy/configuration_and_development/configuration_guide/others/configuring_amcl/)), trouver le paramètre qui définit le modèle de mouvement du robot (dans le bloc `amcl: ros__parameters:` de `nav2_params.yaml`), et remplacer sa valeur par celle correspondant à un robot omnidirectionnel/holonome.

**2. Configurer `global_costmap` et `local_costmap`.**

Un costmap est une version de la carte utilisée pour la navigation : chaque cellule a une valeur représentant la difficulté/le danger à la traverser (0 = libre, 254 = obstacle). Les algorithmes de planification évitent les zones de coût élevé, un peu comme on évite les reliefs en montagne pour choisir le chemin le plus "plat".

Le **`global_costmap`** couvre toute la carte connue et sert à planifier l'itinéraire global (comme une carte routière pour un trajet longue distance). Le **`local_costmap`** ne couvre qu'une petite zone autour du robot et sert à l'évitement d'obstacles en temps réel, y compris ceux qui ne sont pas sur la carte (comme la conduite dans son quartier, où l'on réagit aux voitures garées ou aux piétons). Les deux sont composés de **couches** (`plugins`), chacune apportant un type d'information au costmap.

Ce que fait chaque couche (`plugin`) possible :
- **`static_layer`** : charge la carte connue (celle produite par le SLAM/`map_server`). C'est la référence fixe de l'environnement.
- **`obstacle_layer`** : marque les obstacles détectés par un capteur 2D, typiquement un LIDAR (`/scan`).
- **`voxel_layer`** : équivalent 3D de l'`obstacle_layer`, pour un capteur comme une caméra de profondeur (`PointCloud2`).
- **`inflation_layer`** : ajoute une marge de sécurité autour des obstacles détectés par les autres couches.
- **`denoise_layer`** : filtre le bruit dans les données capteur brutes (points isolés/parasites, ex. sur un scan LIDAR).

À toi de décider, pour `global_costmap` et pour `local_costmap` séparément, quelles couches ont du sens — en te basant sur ce que fait chacune (ci-dessus), ce que sert à faire chaque costmap (global/local, vu plus haut), et les capteurs réellement disponibles sur le X3 dans ce TP (un LIDAR 2D, pas de caméra de profondeur).

Il faut aussi adapter **`robot_radius`** (dans les deux) : le fichier de base utilise la taille d'un TurtleBot, pas celle du X3. Les dimensions réelles du châssis sont dans `src/yahboom_rosmaster/yahboom_rosmaster_description/urdf/mech/rosmaster_x3_base.urdf.xacro` (propriétés `base_length`, `total_width`) — à partir de ça, calculer un rayon englobant raisonnable.

Documentation Nav2 pour ces paramètres : [configuration des costmaps](https://docs.nav2.org/jazzy/configuration_and_development/configuration_guide/core_servers/costmap_2d/) (paramètres généraux, dont `robot_radius`), et les pages des couches individuelles qui y sont liées (`static`, `obstacle`, `inflation`, `voxel`) — pour trouver comment configurer chaque couche choisie (ex. le nom exact du `plugin`, les paramètres de son bloc `ros__parameters`).

**3. Configurer `bt_navigator`.**

Le `bt_navigator` exécute l'arbre de comportement (*behavior tree*) qui orchestre la navigation : il appelle tour à tour le calcul de trajectoire, le suivi de trajectoire, et les comportements de récupération en cas d'échec. Pour certaines de ces décisions, il a besoin de connaître la vitesse courante du robot, qu'il récupère en s'abonnant à un topic d'odométrie.

Par défaut, ce topic est l'odométrie brute des roues. Chez nous, on dispose d'une odométrie fusionnée par l'EKF (section *Localisation et fusion de capteurs*), plus fiable :
```yaml
odom_topic: /odometry/filtered  # au lieu de /odom
```

**4. Configurer `controller_server`.**

Le `controller_server` calcule, en temps réel, les commandes de vitesse (`cmd_vel`) à envoyer au robot pour suivre la trajectoire planifiée. Le contrôleur utilisé par défaut est le MPPI (*Model Predictive Path Integral*) : il simule un grand nombre de trajectoires possibles sur un court horizon temporel et choisit la meilleure selon plusieurs critères (rester sur le chemin, éviter les obstacles, atteindre le but, etc.).

Comme pour `amcl`, le fichier de base suppose un robot différentiel. Plusieurs points à changer pour un robot mecanum (holonome, peut se déplacer latéralement) :

```yaml
odom_topic: "odometry/filtered"   # même raison que pour bt_navigator

min_y_velocity_threshold: 0.001   # 0.5 n'a de sens que pour un robot sans vitesse latérale, le X3 en a une

motion_model: "Omni"
```

Le bloc `AckermannConstraints` (sous `FollowPath`) peut être supprimé : il ne s'applique qu'aux modèles `DiffDrive`/Ackermann.

Les autres paramètres du MPPI (`vx_max`, `vy_max`, `wz_max`, poids des critics, etc.) sont des réglages fins à ajuster en testant la navigation réelle — pas de changement obligatoire ici.

**5. Configurer `velocity_smoother`.**

Le `velocity_smoother` lisse les commandes de vitesse produites par le `controller_server` avant de les envoyer au robot, pour éviter des changements de vitesse trop brusques (accélérations/décélérations limitées).

Même changement que pour `bt_navigator` :
```yaml
odom_topic: "odometry/filtered"  # au lieu de "odom"
```

Mais il y a un piège plus sérieux : `max_velocity`, `min_velocity`, `max_accel` et `max_decel` sont des tableaux `[x, y, theta]`, et le fichier de base met la composante `y` (latérale) à `0.0` — logique pour un robot différentiel, qui ne peut pas se déplacer sur le côté. Pour le X3 (mecanum), laisser cette composante à `0` écraserait à zéro toute vitesse latérale calculée par le `controller_server` (pourtant configuré en `motion_model: "Omni"`, étape précédente) juste avant qu'elle n'atteigne le robot — la navigation fonctionnerait, mais sans jamais utiliser le déplacement latéral. Il faut donc aussi autoriser une vitesse/accélération non nulle sur `y` :
```yaml
max_velocity: [0.5, 0.5, 2.5]
min_velocity: [-0.5, -0.5, -2.5]
max_accel: [0.3, 0.3, 3.2]
max_decel: [-0.5, -0.5, -3.2]
```
> Vérifié en pratique : en publiant une commande `Twist` avec une composante `y` non nulle sur `/cmd_vel_nav` (l'entrée du `velocity_smoother`), elle ressort bien non nulle sur `/cmd_vel_smoothed` avec ces valeurs — alors qu'avec `y: 0.0` dans `max_velocity`/`max_accel`, elle serait écrasée à `0.0`.

**6. `planner_server`, `smoother_server`, `behavior_server`, `collision_monitor` : rien à changer.**

Ces quatre blocs restent identiques au fichier par défaut officiel de Nav2 :
- **`planner_server`** : calcule le chemin global sur le `global_costmap` (algorithme `GridBased`/NavFn). Ne dépend pas du type de déplacement du robot — fonctionne pareil pour un robot différentiel ou mecanum.
- **`smoother_server`** : lisse le chemin global avant de le transmettre au `controller_server`, pour des trajectoires plus naturelles. Même raison, rien de spécifique au mecanum.
- **`behavior_server`** : gère les comportements de récupération (`Spin`, `BackUp`, `DriveOnHeading`, `Wait`, `AssistedTeleop`) en cas d'échec de la navigation. Tourner sur place ou reculer reste valide pour un robot holonome.
- **`collision_monitor`** : surveille les collisions en continu à partir de l'empreinte réelle du robot (`footprint_topic`) et du LIDAR, pas d'une zone directionnelle fixe — ça fonctionne donc indépendamment du type de déplacement.

**7. Lancer la navigation complète.**

Même principe que pour la localisation (section *Localisation et fusion de capteurs*) : le launch file et le script qui combinent tout (Gazebo + EKF + Nav2 + relais `cmd_vel`) vivent dans `yahboom_rosmaster_bringup`, déjà fournis — `launch/rosmaster_x3_navigation.launch.py` et `scripts/rosmaster_x3_navigation.sh` — expliqués en live. Son cœur est simple : trois `IncludeLaunchDescription` (Gazebo, EKF, Nav2) plus le nœud du relais, chacun ajouté à la même `LaunchDescription` — pas de `sleep` nécessaire, chaque nœud attend ses propres dépendances via la découverte ROS (même logique que pour la localisation).

Quelques points qui ont posé problème en pratique, à garder en tête pour l'explication en live :
- **Monde Gazebo** : le launch file doit bien transmettre `world_file` (et `x`/`y`/`z`/`roll`/`pitch`/`yaw`) à l'inclusion du launch Gazebo. Sans ça, Gazebo retombe silencieusement sur son monde par défaut (`empty.world`) au lieu de `cafe.world`.
- **Casse des booléens `slam`/`use_composition`** : `bringup_launch.py` (dans `nav2_bringup`) évalue ces deux arguments comme du code Python (`PythonExpression`), donc il faut passer `True`/`False` (majuscule), pas `true`/`false` — contrairement aux autres arguments booléens (`use_sim_time`, etc.).
- **`use_composition:=False`** : évite une race condition connue entre `pluginlib`/`class_loader` au chargement des plugins Nav2 en composition.
- **`docking_server`** : voir la section *Construire une carte de l'environnement en utilisant le SLAM* (*Supprimer les blocs inutilisés*) — `nav2_bringup` le démarre toujours, il faut donc lui donner un vrai plugin (pas une liste vide).
- **`enable_odom_tf:=false`** : l'EKF (`publish_tf: true` dans `ekf.yaml`) est déjà responsable de la transformée `odom → base_footprint` ; il ne faut pas que le contrôleur ROS 2 la publie aussi (double publication de la même transformée).

Le mode par défaut (construire la carte avec le SLAM) a déjà été vu dans la section précédente. Ici, ce qui nous intéresse est le mode `localize`, pour naviguer sur la carte déjà enregistrée :
```bash
colcon build
source install/setup.bash
bash src/yahboom_rosmaster/yahboom_rosmaster_bringup/scripts/rosmaster_x3_navigation.sh localize
```

**8. Configurer RViz pour visualiser la navigation.**

Le launch file réutilise la configuration RViz de base (`yahboom_rosmaster_gazebo_sim.rviz`, celle des sections précédentes) : elle affiche le robot et les capteurs, mais n'a aucun affichage Nav2 (costmaps, particules AMCL, bouton d'envoi d'objectif). Le tutoriel original d'Addison Sears-Collins référence son propre fichier dédié (`rviz/nav2_default_view.rviz`), mais ce fichier n'a jamais été inclus dans le dépôt public — on ajoute donc les affichages à la main :

- Champ **Fixed Frame** (panneau Displays) : mettre `map` (on raisonne maintenant par rapport à la carte, plus seulement par rapport à l'odométrie).
- **Panels** → **Add New Panel** → **Navigation 2** : ajoute le panneau de contrôle Nav2 (boutons Startup/Pause/Resume, pose initiale, objectif de navigation).
- **Add** → onglet **By topic** → `/map` (`nav_msgs/OccupancyGrid`) : affiche la carte construite par le SLAM (ou chargée, en mode `localize`).
- **Add** → onglet **By topic** → `/global_costmap/costmap` et `/local_costmap/costmap` (`nav2_msgs/Costmap`) : affiche les costmaps (plus haut dans cette section, *Configurer `global_costmap` et `local_costmap`*).
- En mode `localize` (`amcl` actif, pas de SLAM) uniquement : **Add** → onglet **By topic** → `/particle_cloud` (`nav2_msgs/ParticleCloud`) : affiche le nuage de particules de l'AMCL.
- **Tools** (barre d'outils en haut de la vue 3D) → **+** → **Nav2 Goal** : permet de cliquer sur la carte pour envoyer un objectif de navigation ponctuel (alternative au panneau Navigation 2).

**9. Initialiser la position du robot (2D Pose Estimate).**

En mode `localize`, le robot ne connaît pas sa position de départ sur la carte chargée : `amcl` a besoin d'un point de départ pour commencer à affiner son estimation (en mode SLAM, pas besoin — `slam_toolbox` localise le robot dès le début).

- Dans la barre d'outils de la vue 3D RViz, cliquer sur **2D Pose Estimate**.
- Cliquer sur la carte à l'endroit où se trouve réellement le robot dans Gazebo, maintenir le clic gauche et faire glisser la souris dans la direction où il est orienté, puis relâcher.
- `amcl` utilise cette pose comme point de départ et affine ensuite continuellement son estimation à partir du LIDAR et des commandes de mouvement — observable via le nuage `/particle_cloud` (étape précédente) qui se resserre progressivement autour de la position réelle.

> À refaire chaque fois qu'on relance la navigation en mode `localize`, ou si le robot est déplacé manuellement dans Gazebo.

**10. Envoyer un objectif de navigation (Nav2 Goal).**

Une fois la pose initiale fixée (en mode `localize`) — ou directement en mode SLAM, puisque `slam_toolbox` localise déjà le robot :

- **Tools** → **+** → **Nav2 Goal** (ajouté à l'étape *Configurer RViz pour visualiser la navigation*).
- Cliquer sur la carte à l'endroit visé, maintenir le clic gauche et faire glisser pour fixer l'orientation d'arrivée, puis relâcher.
- `bt_navigator` fait planifier un chemin par `planner_server` en évitant les obstacles (costmaps), puis `controller_server` et le relais `cmd_vel` le suivent — observable en direct dans RViz (chemin planifié, costmaps, position qui avance).

**11. Envoyer une séquence de waypoints.**

Pour faire suivre une série de points au lieu d'un seul objectif :

- Fixer la pose initiale si besoin (étape *Initialiser la position du robot*).
- Dans le panneau Nav2, activer le bouton **Waypoint / Nav Through Poses Mode**.
- Cliquer **Nav2 Goal**, puis sur la carte pour chaque point voulu (chacun est étiqueté `wp_#`) ; répéter autant de fois que voulu.
- **Start Waypoint Following** : le robot s'arrête à chaque waypoint avant de continuer vers le suivant. **Start Nav Through Poses** : il les traverse sans s'arrêter.
