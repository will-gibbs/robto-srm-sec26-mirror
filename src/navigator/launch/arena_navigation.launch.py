from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    # 1. Path setup
    nav_pkg_dir = get_package_share_directory('navigator')
    nav2_dir = get_package_share_directory('nav2_bringup')
    slam_dir = get_package_share_directory('slam_toolbox')
    depthai_dir = get_package_share_directory('depthai_ros_driver')

    # 2. Static TF
    static_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        arguments=['--x', '0.1524', '--y', '0', '--z', '0.2108', 
                   '--yaw', '-1.57', '--pitch', '0', '--roll', '-1.57', 
                   '--frame-id', 'base_link', 
                   '--child-frame-id', 'oak_rgb_camera_optical_frame']
    )

    # 3. Camera (OAK-D)
    camera = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(depthai_dir, 'launch', 'pointcloud.launch.py'))
    )

    # 4. Your Node (The Converter)
    nav_node = Node(
        package='navigator',
        executable='nav_node',
        name='camera_navigator',
        output='screen'
    )

    # 5. SLAM Toolbox
    slam = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(slam_dir, 'launch', 'online_async_launch.py'))
    )

    # 6. Nav2 - Notice the CAPITAL 'False'
    nav2 = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(nav2_dir, 'launch', 'navigation_launch.py')),
        launch_arguments={'use_sim_time': 'False'}.items()
    )

    # 7. Dummy Odom
    dummy_odom = Node(
        package='navigator',
        executable='dummy_odom.py',
        name='dummy_odom'
    )

    return LaunchDescription([
        static_tf,
        camera,
        nav_node,
        slam,
        nav2,
        dummy_odom
    ])