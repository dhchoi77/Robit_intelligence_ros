from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(package='my_cpp_pkg', executable='my_pub'),
        Node(package='demo_nodes_cpp', executable='talker'),      # 하나 더
        Node(package='demo_nodes_cpp', executable='listener'),    # 또 하나
    ])
