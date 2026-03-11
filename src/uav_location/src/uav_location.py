###############################################################################
# Project:      Robto SRM Scuffbot, IEEE SoutheastCon 2026                    #
# Package:      uav_location                                                  #
# Name:         uav_location.py                                               #
# Author:       Jonathan Tyler                                                #
# Date Written: 2026-03-11                                                    #
###############################################################################

###############################################################################
# This program is a publisher node that detects the infrared signals sent     #
# from the UAV for location. It publishes two boolean values, each            #
# corresonding to on of the transmitters.                                     #
###############################################################################

import rclpy
from rclpy.node import Node
from std_msgs.msg import BoolMultiArray


class UAVLocation(Node):

    def __init__(self):
        super().__init__("uav_location")

        # Publisher
        self.publisher = self.create_publisher(
            BoolMultiArray,
            "uav_location",
            10
        )

        # Publish at 1 Hz
        self.timer = self.create_timer(
            1.0,
            self.publish_location
        )

        self.get_logger().info("UAV Location publisher started")

    def publish_location(self):

        msg = BoolMultiArray()

        # Arbitrary placeholder values
        msg.data = [True, False]

        self.publisher.publish(msg)

        self.get_logger().info(f"Published: {msg.data}")


def main(args=None):

    rclpy.init(args=args)

    node = UAVLocation()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass

    node.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()