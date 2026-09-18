from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess, TimerAction


def generate_launch_description():
    # 1) 두 노드 실행 (아직 Unconfigured 상태로 뜸)
    fake_imu = Node(
        package='imu_watchdog_pkg',
        executable='fake_imu',
        name='fake_imu',
        output='screen',
        parameters=[{'rate_hz': 50.0, 'stamp_offset_sec': 0.0}],
    )

    watchdog = Node(
        package='imu_watchdog_pkg',
        executable='imu_watchdog',
        name='imu_watchdog',
        output='screen',
        parameters=[{'timeout_sec': 0.5, 'check_rate_hz': 10.0}],
    )

    # 2) lifecycle 전이 명령들 (순서대로 configure → activate)
    def lc(node, transition):
        return ExecuteProcess(
            cmd=['ros2', 'lifecycle', 'set', f'/{node}', transition],
            output='screen',
        )

    # 3) 노드가 뜰 시간을 주고 나서 전이 (타이머로 지연)
    configure = TimerAction(period=2.0, actions=[
        lc('fake_imu', 'configure'),
        lc('imu_watchdog', 'configure'),
    ])
    activate = TimerAction(period=3.5, actions=[
        lc('fake_imu', 'activate'),
        lc('imu_watchdog', 'activate'),
    ])

    return LaunchDescription([fake_imu, watchdog, configure, activate])