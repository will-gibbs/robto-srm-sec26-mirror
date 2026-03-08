###############################################################################
# Project:      Robto SRM Scuffbot, IEEE SoutheastCon 2026                    #
# Package:      uav_controller                                                #
# Name:         uav_controller.py                                             #
# Author:       Jonathan Tyler                                                #
# Date Written: 2026-03-08                                                    #
###############################################################################

###############################################################################
# This program is the controller for Robtina, Robto's UAV assistant.          #
#                                                                             #
# The UAV has a position and velocity as follows:                             #
# Position:                                                                   #
# - x: Primary horizontal dimension; for the world frame, positive is east;   #
#      for a vehicle frame, positive is forward                               #
# - y: Secondary horizontal dimension; for the world frame, positive is       #
#      north; for a vehicle frame, positive is right                          #
# - z: Vertical position; positive is down, consistent with aviation          #
#      convention; the UAV begins at altitude 0                               #
# - orientation: Angle in radians in standard position for a Cartesian plane  #
#      (0 is the positive x axis, PI/2 is the positive y axis, etc.)          #
# Velocity:                                                                   #
# - u: Forward velocity (x axis)                                              #
# - v: Lateral velocity (y axis)                                              #
# - w: Vertical velocity (z axis)                                             #
# - yaw: Angular velocity of the orientation                                  #
#                                                                             #
# Primary navigation for the UAV consists of keeping a queue of waypoints.    #
# The UAV will set its velocity to go toward the waypoint at the front of the #
# queue. Once it reaches the waypoint, it removes it from the queue. If the   #
# queue is empty, the UAV will hover.                                         #
#                                                                             #
# The UAV controller has a timer that sends navigation commands to the radio  #
# transmitter. At each tick, the controller computes the UAV's current        #
# position based on its past position and current velocity. Then, it uses the #
# difference between the current position and the target waypoint to compute  #
# a new velocity and send it to the transmitter.                              #
###############################################################################

# Imports
from src.sec_core_classes import Controller
from src.sec_interfaces.action import CompleteTask

import rclpy
from rclpy.action import GoalResponse, CancelResponse
import threading

# Constants
#? Temporary values until testing
MAX_VERTICAL_VELOCITY = 100
MAX_FORWARD_VELOCITY = 100
MAX_LATERAL_VELOCITY = 100
TICK_RATE = 0.03 # in seconds

###############################################################################
# UAV Controller Class Definition                                             #
###############################################################################
class UavController(Controller):
   # Constructor
   def __init__(self):
      #? don't forget the general Controller data members
      self.position_in_world = self.get_rover_position()
      self.position_in_rover = Position(0, 0, 0, 0)
      self.velocity = Velocity(0, 0, 0, 0)
      self.timer = self.create_timer(TICK_RATE, self.timer_callback)
      pass

   # Get the rover's positions in the world frame
   def get_rover_position():
      pass

   # Respond to receive an action goal request
   def handle_goal(self, goal_request):
      self.get_logger().info("Received a request to complete the UAV task.")
      if goal_request.begin_task == goal_request.BEGIN_TASK:
         self.get_logger().info("Request accepted. Executing the UAV task.")
         return GoalResponse.ACCEPT
      else:
         self.get_logger().info("Request denied. Invalid request code.")
         return GoalResponse.REJECT
      
   # Respond to a request to cancel a goal
   def handle_cancel(self, goal_handle):
      self.get_logger().info("Received cancel UAV task request.")
      self.get_logger().info("UAV task goal canceled.")
      return CancelResponse.ACCEPT

   # Respond to accepting a request
   def handle_accepted(self, goal_handle):
      thread = threading.Thread(
         target=self.execute,
         args=(goal_handle,),
         daemon=True
      )
      thread.start()

   # Execute the UAV task
   def execute():
      # Launch the UAV
      # Move the required distance away from the rover
      # Move back to the roer
      # Land the UAV back on the rover
      # First step completed
      # Options: Hover over the rover, or hover by the earth module
      # Relay color data as needed
      # Land the UAV (optional)
      pass

   # Every timer tick, calculate the necessary commands to send to the rover
   def timer_callback():
      pass

   def launch():
      pass
   def goto(world_position):
      pass



###############################################################################
# Position in a three-dimensional coordinate system                           #
###############################################################################
class Position:
   # Constructor
   def __init__(self, x, y, z, orientation):
      self.__x = x
      self.__y = y
      self.__z = z
      self.__orientation = orientation

   # Get the data members
   def get_x(self):
      return self.__x
   def get_y(self):
      return self.__y
   def get_z(self):
      return self.__z
   def get_orientation(self):
      return self.__orientation
   
   # Set the data members
   def set_x(self, x):
      self.__x = x
   def set_y(self, y):
      self.__y = y
   def set_z(self, z):
      self.__z = z
   def set_orientation(self, orientation):
      self.__orientation = orientation
   


###############################################################################
# Velocity of the UAV                                                         #
###############################################################################
class Velocity:
   # Constructor
   def __init__(self, u, v, w, yaw):
      self.__u = u     # Forward velocity
      self.__v = v     # Lateral velocity
      self.__w = w     # Vertical velocity
      self.__yaw = yaw # Yaw, or spin velocity

   # Get the data members
   def get_u(self):
      return self.__u
   def get_v(self):
      return self.__v
   def get_w(self):
      return self.__w
   def get_yaw(self):
      return self.__yaw
   
   # Set the data members
   def set_u(self, u):
      self.__u = u
   def set_v(self, v):
      self.__v = v
   def set_w(self, w):
      self.__w = w
   def set_yaw(self, yaw):
      self.__yaw = yaw