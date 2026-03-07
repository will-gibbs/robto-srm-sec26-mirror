from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    # 1. Static Transform: Links base_link (ground center) to the Camera
    # Arguments: x y z yaw pitch roll frame_id child_frame_id
    # We use 0.1524m forward and 0.2108m up. 
    # Note: We assume the camera is facing straight forward (0 rad rotation).
    static_tf = Node(
    package='tf2_ros',
    executable='static_transform_publisher',
    # Arguments: x y z yaw pitch roll frame_id child_frame_id
    # We rotate -1.57 on Yaw and 1.57 on Pitch to align Optical to Base Link
    arguments=['0.1524', '0', '0.2108', '-1.57', '0', '-1.57', 'base_link', 'oak_rgb_camera_optical_frame']
    )

    # 2. OAK-D Camera Driver (DepthAI)
    camera_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            get_package_share_directory('depthai_ros_driver'), 'launch', 'pointcloud.launch.py'
        ))
    )

    # 3. Your C++ Converter & Navigator Node
    nav_node = Node(
        package='my_nav_pkg',
        executable='nav_node',
        name='camera_navigator',
        output='screen'
    )

    # 4. SLAM Toolbox (Creates the map/arena from your LaserScan)
    slam_toolbox = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            get_package_share_directory('slam_toolbox'), 'launch', 'online_async_launch.py'
        ))
    )

    # 5. Nav2 Bringup (Handles path planning and obstacle avoidance)
    nav2_bringup = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            get_package_share_directory('nav2_bringup'), 'launch', 'navigation_launch.py'
        )),
        launch_arguments={'use_sim_time': 'false'}.items()
    )

    dummy_odom = Node(
        package='my_nav_pkg',
        executable='dummy_odom.py',
        name='dummy_odom'
    )

    return LaunchDescription([
        static_tf,
        camera_launch,
        nav_node,
        slam_toolbox,
        nav2_bringup,
        dummy_odom
    ])