# depthimage_to_pointcloud2
Modified from https://github.com/ros2/turtlebot2_demo/tree/a612ef2b9b63ba9d19513b67c1d6e5eede680cdf

## Usage examples 

### [Command line remapping](https://docs.ros.org/en/galactic/How-To-Guides/Node-arguments.html):
```
ros2 run depthimage_to_pointcloud2 depthimage_to_pointcloud2_node --ros-args -r depth:=/my_depth_sensor/image -r depth_camera_info:=/my_depth_sensor/camera_info -r pointcloud2:=/my_output_topic -r __node:=my_new_node_name
```

### [Launch file](https://docs.ros.org/en/galactic/Tutorials/Launch/Creating-Launch-Files.html?highlight=remappings):
#### Simple
```
from launch import LaunchDescription
from launch_ros.actions import Node
def generate_launch_description():
    return LaunchDescription([Node(
        package="depthimage_to_pointcloud2",
        executable="depthimage_to_pointcloud2_node",
        output='screen',
        name=`my_node_name`,
        remappings=[
            ("depth", "/my_depth_sensor/image"),
            ("depth_camera_info", "/my_depth_sensor/camera_info"),
            ("pointcloud2", "/my_output_topic")
            ]
        )
    ])
```

#### Complex (using the annoying `PythonExpression`)
```
from launch.substitutions import LaunchConfiguration, PythonExpression

from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            'full_sensor_topic',
            default_value=['/my_depth_sensor'],
            description='Base for topic (and node) names'),
        Node(
        package="depthimage_to_pointcloud2",
        executable="depthimage_to_pointcloud2_node",
        output='screen',
        name=[PythonExpression(["'", LaunchConfiguration('full_sensor_topic'), "'.split('/')[-1]"]), '_depth2pc2'],
        remappings=[
            ("depth", [LaunchConfiguration('full_sensor_topic'), "/image"]),
            ("depth_camera_info", [LaunchConfiguration('full_sensor_topic'), "/camera_info"]),
            ("pointcloud2", [PythonExpression(["'", LaunchConfiguration('full_sensor_topic'), "'.split('/')[-1]"]), "_pointcloud2"])
            ]
        )
    ])
```

And considering my `full_sensor_topic` is `/my_weird/depth_sensor`:
```
$ ros2 launch depthimage_to_pointcloud2 depthimage_to_pointcloud2.launch.py full_sensor_topic:=/my_weird/depth_sensor
```

It will be looking at the topics:
```
/my_weird/depth_sensor/image
/my_weird/depth_sensor/camera_info
```
And publishing the pointcloud2:
```
/depth_sensor_pointcloud2
```
Finally, the node name will also be `/depth_sensor_pointcloud2`.