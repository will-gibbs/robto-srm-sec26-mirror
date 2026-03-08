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

###############################################################################
# UAV Controller Class Definition                                             #
###############################################################################
class UavController(Controller):
   # Constructor
   def UavController(self):
      self.position_in_world = get_rover_position()
      self.position_in_rover = Position(0, 0, 0, 0)
      self.velocity = Velocity(0, 0, 0, 0)
      pass
   def get_rover_position():
      pass

###############################################################################
# Position in a three-dimensional coordinate system                           #
###############################################################################
class Position:
   def Position(self, x, y, z, orientation):
      self.__x = x
      self.__y = y
      self.__z = z
      self.__orientation = orientation
   def get_x(self):
      return self.__x
   def get_y(self):
      return self.__y
   def get_z(self):
      return self.__z
   def get_orientation(self):
      return self.__orientation
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
   def Velocity(self, u, v, w, yaw):
      self.__u = u     # Forward velocity
      self.__v = v     # Lateral velocity
      self.__w = w     # Vertical velocity
      self.__yaw = yaw # Yaw, or spin velocity
   def get_u(self):
      return self.__u
   def get_v(self):
      return self.__v
   def get_w(self):
      return self.__w
   def get_yaw(self):
      return self.__yaw
   def set_u(self, u):
      self.__u = u
   def set_v(self, v):
      self.__v = v
   def set_w(self, w):
      self.__w = w
   def set_yaw(self, yaw):
      self.__yaw = yaw