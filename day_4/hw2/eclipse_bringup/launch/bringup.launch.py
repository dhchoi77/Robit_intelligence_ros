from launch import LaunchDescription
from launch.substitutions import Command, FindExecutable, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    # ── ① xacro를 arg 켜서 실행 → URDF 생성 ──
    robot_description_content = Command(
        [
            PathJoinSubstitution([FindExecutable(name="xacro")]),
            " ",
            PathJoinSubstitution(
                [FindPackageShare("robot_description"), "urdf", "eclipse.xacro"]
            ),
            # ↓ 여기가 display launch와 결정적으로 다른 부분!
            " with_base:=true",
            " with_arm:=true",
            " with_camera_tower:=true",
            " use_mock_base:=true",
            " use_mock_arm:=true",
            " use_mock_ct:=true",
        ]
    )
    robot_description = {"robot_description": ParameterValue(
        robot_description_content, value_type=str
    )}
    # ── ② robot_state_publisher ──
    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="screen",
        parameters=[robot_description],
    )
    # ── ③ controller_manager (ros2_control_node) ──
    controllers_yaml = PathJoinSubstitution(
        [FindPackageShare("eclipse_bringup"), "config", "controllers.yaml"]
    )
    control_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        output="screen",
        parameters=[robot_description, controllers_yaml],
    )
    # ── ④ spawner들: 컨트롤러 activate ──
    jsb_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster"],
        output="screen",
    )
    arm_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["arm_controller"],
        output="screen",
    )
    ct_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["camera_tower_controller"],
        output="screen",
    )
    diff_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["diff_drive_controller"],
        output="screen",
    )
    flipper_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["flipper_controller"],
        output="screen",
    )
    # ── ⑤ rviz2 (일단 설정 없이) ──
    rviz = Node(
        package="rviz2",
        executable="rviz2",
        output="screen",
    )

    return LaunchDescription([
        robot_state_publisher,
        control_node,
        jsb_spawner,
        arm_spawner,
        ct_spawner,
        diff_spawner,
        flipper_spawner,
        rviz,
    ])