from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='my_cpp_pkg',
            executable='my_pub',
            name='my_pub',
        ),
        Node(
            package='my_cpp_pkg',
            executable='my_sub',
            name='my_sub',
        ),
    ])
