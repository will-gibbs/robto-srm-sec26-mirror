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
MAX_VERTICAL_SPEED = 100.0
MAX_HORIZONTAL_SPEED = 100.0
MAX_YAW_RATE = 1.0
MAX_VERTICAL_ACCELERATION = 0.1
MAX_HORIZONTAL_ACCELERATION = 0.1
MAX_YAW_ACCELERATION = 0.1
VERTICAL_GAIN = 0.5           # Proportional gain on the z axis
HORIZONTAL_GAIN = 0.5         # Proportional gain on the x and y axes
ANGULAR_GAIN = 1.2            # Proportional gain for angular yaw velocity
TICK_RATE = 0.03              # In seconds

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

      self.velocity_in_body = self.world_to_body(self.velocity_in_world)

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

   # Calculate a new velocity based on the distance to the target position
   def calculate_new_velocity(self):
      # If there is no waypoint, hover
      if not self.waypoints:
         self.velocity_in_world.u = 0.0
         self.velocity_in_world.v = 0.0
         self.velocity_in_world.w = 0.0
         self.velocity_in_world.yaw_rate = 0.0
         return

      # Calculate a new yaw rate
      yaw_error = self.waypoints[0].yaw - self.position_in_world.yaw
      yaw_error = math.atan2(math.sin(yaw_error), math.cos(yaw_error))

      # Define the difference between the current and target position
      self.position_error = Position(
         x=self.waypoints[0].x - self.position_in_world.x,
         y=self.waypoints[0].y - self.position_in_world.y,
         z=self.waypoints[0].z - self.position_in_world.z,
         yaw=yaw_error
      )

      # Set velocity components to be proportions of error components
      vx = HORIZONTAL_GAIN * self.position_error.x
      vy = HORIZONTAL_GAIN * self.position_error.y
      vz = VERTICAL_GAIN * self.position_error.z
      yaw_rate = ANGULAR_GAIN * self.position_error.yaw

      # Cap speeds
      horizontal_speed = math.hypot(vx*vx + vy*vy)
      if horizontal_speed > MAX_HORIZONTAL_SPEED:
         scale = MAX_HORIZONTAL_SPEED / horizontal_speed
         vx *= scale
         vy *= scale
      vz = clamp(vz, -MAX_VERTICAL_SPEED, MAX_VERTICAL_SPEED)
      yaw_rate = clamp(yaw_rate, -MAX_YAW_RATE, MAX_YAW_RATE)

      # Cap accelerations
      max_xy_accel = MAX_HORIZONTAL_ACCELERATION * TICK_RATE
      max_z_accel = MAX_VERTICAL_ACCELERATION * TICK_RATE
      max_yaw_accel = MAX_YAW_ACCELERATION * TICK_RATE
      vx, vy = limit_horizontal_acceleration(
         self.velocity_in_world.u,
         self.velocity_in_world.v,
         vx,
         vy,
         max_xy_accel
      )
      vz = limit_acceleration(self.velocity_in_world.w, vz, max_z_accel)
      yaw_rate = limit_acceleration(self.velocity_in_world.yaw_rate, yaw_rate, max_yaw_accel)

      # Store the components of the new velocity
      self.velocity_in_world.u = vx
      self.velocity_in_world.v = vy
      self.velocity_in_world.w = vz
      self.velocity_in_world.yaw_rate = yaw_rate

   # Convert a world velocity to a UAV velocity
   def world_to_body():
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



###############################################################################
# Functions                                                                   #
###############################################################################
# Clamp a value between two boundaries
def clamp(value, low, high):
   return max(low, min(high, value))

# Move toward a target velocity with an acceleration cap
def limit_acceleration(current_velocity, target_velocity, max_acceleration):
   acceleration = target_velocity - current_velocity
   acceleration = clamp(acceleration, -max_acceleration, max_acceleration)
   return current_velocity + acceleration

# Move toward a target two-dimensional velocity with a horizontal velocity cap
def limit_horizontal_acceleration(current_vx, current_vy, target_vx, target_vy, max_acceleration):
    ax = target_vx - current_vx
    ay = target_vy - current_vy

    acceleration = math.hypot(ax, ay)
    if acceleration > max_acceleration:
        scale = max_acceleration / acceleration
        ax *= scale
        ay *= scale

    return current_vx + ax, current_vy + ay