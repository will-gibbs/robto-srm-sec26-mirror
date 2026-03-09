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
# - yaw: Angle in radians in standard position for the world frame            #
#      (0 is the positive x axis, PI/2 is the positive y axis, etc.)          #
# Velocity:                                                                   #
# - u: Forward velocity (x axis)                                              #
# - v: Lateral velocity (y axis)                                              #
# - w: Vertical velocity (z axis)                                             #
# - yaw_rate: Angular velocity of the yaw                                     #
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

import math
import threading
from dataclasses import dataclass

# Constants
#? Temporary values until testing
MAX_VERTICAL_VELOCITY = 100
MAX_FORWARD_VELOCITY = 100
MAX_LATERAL_VELOCITY = 100
VERTICAL_GAIN = 0.5          # Proportional gain on the z axis
HORIZONTAL_GAIN = 0.5        # Proportional gain on the x and y axes
ANGULAR_GAIN = 1.2           # Proportional gain for angular yaw velocity
TICK_RATE = 0.03             # In seconds

###############################################################################
# UAV Controller                                                              #
###############################################################################
class UavController(Controller):
   # Constructor
   def __init__(self):
      #? don't forget the general Controller data members
      self.uav_is_launched
      self.position_in_world = self.get_rover_position()
      self.position_in_rover = Position(0, 0, 0, 0)
      self.velocity_in_body = Velocity(0, 0, 0, 0)
      self.velocity_in_world = Velocity(0, 0, 0, 0)
      self.timer = self.create_timer(TICK_RATE, self.timer_callback)

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

   # Update the current position and send a new velocity command
   def timer_callback(self):
      #? probably need to check if the UAV is on

      self.update_position()
      self.calculate_new_velocity()

      # Send the commands to the radio transmitter

   # Launch the UAV
   def launch():
      pass

   # Navigate to a specified position
   def goto(world_position):
      pass

   # Update the current position of the UAV after a timer tick
   def update_position(self):
      dt = TICK_RATE
      omega = self.velocity_in_world.yaw_rate
      dtheta = omega * dt
      vx = self.velocity_in_world.u
      vy = self.velocity_in_world.v

      if abs(omega) < 1e-6:
         self.position_in_world.x += vx * dt
         self.position_in_world.y += vy * dt
      else:
         self.position_in_world.x += (vx * math.sin(dtheta) + vy * (math.cos(dtheta) - 1)) / omega
         self.position_in_world.y += (vx * (math.cos(dtheta) - 1) + vy * math.sin(dtheta)) / omega

      self.position_in_world.z += self.velocity_in_world.w * dt
      self.position_in_world.yaw += dtheta
      self.position_in_world.yaw = (self.position_in_world.yaw + math.pi) % (2*math.pi) - math.pi

   # Calculate a new velocity
   def calculate_new_velocity(self):
      #? Needs to have velocity caps and add smooth acceleration
      yaw_error = self.waypoints[0].yaw - self.position_in_world.yaw
      self.position_error = Position(
         x=self.waypoints[0].x - self.position_in_world.x,
         y=self.waypoints[0].y - self.position_in_world.y,
         z=self.waypoints[0].z - self.position_in_world.z,
         yaw=math.atan2(math.sin(yaw_error), math.cos(yaw_error))
      )
      self.velocity_in_world.u = HORIZONTAL_GAIN * self.position_error.x
      self.velocity_in_world.v = HORIZONTAL_GAIN * self.position_error.y
      self.velocity_in_world.w = VERTICAL_GAIN * self.position_error.z
      self.velocity_in_world.yaw_rate = ANGULAR_GAIN * self.position_error.yaw
      self.velocity_in_body = self.world_to_uav(self.velocity_in_world)

   # Convert a world velocity to a UAV velocity
   def world_to_uav():
      pass



###############################################################################
# Position in a three-dimensional coordinate system                           #
###############################################################################
@dataclass
class Position:
   x: float = 0.0
   y: float = 0.0
   z: float = 0.0
   yaw: float = 0.0
   


###############################################################################
# Velocity of the UAV                                                         #
###############################################################################
@dataclass
class Velocity:
   u: float = 0.0        # Primary horizontal velocity (x axis)
   v: float = 0.0        # Secondary horizontal velocity (y axis)
   w: float = 0.0        # Vertical velocity (z axis)
   yaw_rate: float = 0.0 # Angular velocity of the UAV's yaw