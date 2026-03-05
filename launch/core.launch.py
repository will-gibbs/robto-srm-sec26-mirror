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
        # arguments=['--ros-args', '--log-level', 'manager:=ERROR']
    )
    controller_test = Node(
        package='controller_test',
        executable='controller_test',
        name='controller_test'
    )
    timekeeper_node = Node(
        package='timekeeper',
        executable='timekeeper',
        name='timekeeper',
        arguments=['--ros-args', '--log-level', 'timekeeper:=ERROR']
    )

    return LaunchDescription([
        manager_node,

        RegisterEventHandler(
            OnProcessStart(
                target_action=manager_node,
                on_start=[
                    LogInfo(msg='Manager started. Starting controller test...'),
                    LogInfo(msg='Test Message'),
                    TimerAction(
                        period=5.0,
                        actions=[controller_test]
                    )
                ]
            )
        ),

        RegisterEventHandler(
            OnProcessStart(
                target_action=controller_test,
                on_start=[
                    LogInfo(msg='Controller test started. Starting timekeeper...'),
                    TimerAction(
                        period=5.0,
                        actions=[timekeeper_node]
                    )
                ]
            )
        )
    ])
