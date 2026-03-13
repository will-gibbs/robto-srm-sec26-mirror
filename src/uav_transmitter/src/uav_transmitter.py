###############################################################################
# Project:      Robto SRM Scuffbot, IEEE SoutheastCon 2026                    #
# Package:      uav_transmitter                                               #
# Name:         uav_transmitter.py                                            #
# Author:       Jonathan Tyler                                                #
# Date Written: 2026-03-13                                                    #
###############################################################################

###############################################################################
# This program is a subscriber node that receive twist (velocity) messages    #
# and converts them into radio signals for the UAV.                           #
#                                                                             #
# Subscriptions:                                                              #
# - uav_velocity                                                              #
###############################################################################

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist

###############################################################################
# UAV radio transmitter node                                                  #
###############################################################################
class UavTransmitter(Node):
   def __init__(self):
      super().__init__('uav_transmitter')

      # Create for velocity messages
      self.subscription = self.create_subscription(
         Twist,                  # Message type
         'uav_velocity',         # Topic name
         self.listener_callback, # Callback function
         10                      # QoS history depth
      )

      # Prevent unused variable warning
      self.subscription

   # Callback to be executed whenever a message is received
   def listener_callback(self, msg: Twist):
      linear = msg.linear
      angular = msg.angular

      self.get_logger().info(
         f"Linear: ({linear.x}, {linear.y}, {linear.z}) | "
         f"Angular: ({angular.x}, {angular.y}, {angular.z})"
      )



###############################################################################
# Main function                                                               #
###############################################################################
def main(args=None):
   rclpy.init(args=args)
   node = UavTransmitter()
   rclpy.spin(node)

   node.destroy_node()
   rclpy.shutdown()

# Standard entry-point guard
if __name__ == '__main__':
   main()