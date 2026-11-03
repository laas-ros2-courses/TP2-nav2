import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    default_ekf_config = os.path.join(
        get_package_share_directory('yahboom_rosmaster_localization'), 'config', 'ekf.yaml')

    use_sim_time = LaunchConfiguration('use_sim_time')
    ekf_config_file = LaunchConfiguration('ekf_config_file')

    declare_use_sim_time = DeclareLaunchArgument(
        name='use_sim_time',
        default_value='true',
        description='Use simulation (Gazebo) clock if true'
    )

    declare_ekf_config_file = DeclareLaunchArgument(
        name='ekf_config_file',
        default_value=default_ekf_config,
        description='Full path to the EKF configuration YAML file'
    )

    ekf_node = Node(
        package='robot_localization',
        executable='ekf_node',
        name='ekf_filter_node',
        output='screen',
        parameters=[ekf_config_file, {'use_sim_time': use_sim_time}]
    )

    return LaunchDescription([
        declare_use_sim_time,
        declare_ekf_config_file,
        ekf_node
    ])
