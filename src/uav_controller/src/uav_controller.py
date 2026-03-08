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
###############################################################################

# Imports
from src.sec_core_classes import Controller
from src.sec_interfaces.action import CompleteTask

import rclpy
from rclpy.action import GoalResponse, CancelResponse

# Constants
#? Temporary values until testing
MAX_VERTICAL_VELOCITY = 100
MAX_FORWARD_VELOCITY = 100
MAX_LATERAL_VELOCITY = 100
TICK_RATE = 10

###############################################################################
# UAV Controller Class Definition                                             #
###############################################################################
class UavController(Controller):
   # Constructor
   def __init__(self):
      #? don't forget the general Controller data members
      self.position_in_world = get_rover_position()
      self.position_in_rover = Position(0, 0, 0, 0)
      self.velocity = Velocity(0, 0, 0, 0)
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
      
   # 
   def handle_cance():
      pass
   def handle_accepted():
      pass
   def execute():
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