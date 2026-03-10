from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    nav2_dir = get_package_share_directory('nav2_bringup')
    nav_pkg_dir = get_package_share_directory('navigator')
    slam_dir = get_package_share_directory('slam_toolbox')
    depthai_dir = get_package_share_directory('depthai_ros_driver')
    
    use_sim_time = 'false'
    params_file = os.path.join(nav_pkg_dir, 'config', 'nav2_params.yaml')
    slam_params_file = os.path.join(nav_pkg_dir, 'config', 'slam_params.yaml')

    # Camera - depthai_ros_driver often uses PythonExpression for conditions
    camera = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(depthai_dir, 'launch', 'pointcloud.launch.py')),
        launch_arguments={'use_sim_time': use_sim_time}.items()
    )

    # SLAM Toolbox (UPDATED)
    slam = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(slam_dir, 'launch', 'online_async_launch.py')),
        launch_arguments={
            'use_sim_time': 'False',
            'slam_params_file': slam_params_file  # <--- Pass your custom config here
        }.items() 
    )

    # Nav2
    nav2 = IncludeLaunchDescription(
    PythonLaunchDescriptionSource(os.path.join(nav2_dir, 'launch', 'navigation_launch.py')),
    launch_arguments={
        'use_sim_time': 'False',
        'params_file': params_file,
        'autostart': 'True',
        'use_composition': 'False',
        'use_lifecycle_manager': 'False', # This must be False
        'map_subscribe_transient_local': 'True'
    }.items()
)

    # Manually create a CLEAN lifecycle manager
    # This ensures collision_monitor is NEVER even attempted
    clean_lifecycle_manager = Node(
        package='nav2_lifecycle_manager',
        executable='lifecycle_manager',
        name='lifecycle_manager_navigation',
        output='screen',
        parameters=[{'use_sim_time': False},
                    {'autostart': True},
                    {'node_names': [
                        'controller_server',
                        'smoother_server',
                        'planner_server',
                        'behavior_server',
                        'bt_navigator',
                        'waypoint_follower',
                        'velocity_smoother'
                    ]}]
    )

    static_tf = Node(
    package='tf2_ros',
    executable='static_transform_publisher',
    # Ensure arguments are strings and match the expected order for Jazzy
    arguments=['0.1524', '0.0', '0.2108', '0', '0', '0', 'base_link', 'oak-d-base-frame']
)

    nav_node = Node(
        package='navigator', executable='nav_node', name='camera_navigator'
    )

    dummy_odom = Node(
        package='navigator', executable='dummy_odom.py', name='dummy_odom'
    )

    return LaunchDescription([
        static_tf, camera, nav_node, slam, nav2, dummy_odom, clean_lifecycle_manager
    ])