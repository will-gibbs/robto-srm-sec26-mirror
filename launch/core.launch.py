from launch import LaunchDescription
from launch.actions import (
    LogInfo,
    RegisterEventHandler,
    TimerAction
)
from launch.event_handlers import OnProcessStart
from launch_ros.actions import Node

def generate_launch_description():
    manager_node = Node(
        package='manager',
        executable='manager',
        name='manager',
        arguments=['--ros-args', '--log-level', 'manager:=ERROR']
    )
    timekeeper_node = Node(
        package='timekeeper',
        executable='timekeeper',
        name='timekeeper'
    )

    return LaunchDescription([
        manager_node,

        RegisterEventHandler(
            OnProcessStart(
                target_action=manager_node,
                on_start=[
                    LogInfo(msg='Manager started. Starting timekeeper...'),
                    TimerAction(
                        period=5.0,
                        actions=[timekeeper_node]
                    )
                ]
            )
        )
    ])
