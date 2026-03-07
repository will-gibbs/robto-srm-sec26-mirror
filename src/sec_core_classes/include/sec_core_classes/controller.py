###############################################################################
# Project:      Robto SRM Scuffbot, IEEE SoutheastCon 2026                    #
# Package:      sec_core_classes                                              #
# Name:         controller.py                                                 #
# Author:       Jonathan Tyler                                                #
# Date Written: 2026-03-07                                                    #
###############################################################################

###############################################################################
# This program is the parent class for Python controllers. Controllers are    #
# designed to complete tasks for the competition. Each controller subclass    #
# will need to override the following functions:                              #
# - handle_goal()                                                             #
# - handle_cancel()                                                           #
# - handle_accepted()                                                         #
# - execute()                                                                 #
# - timer_callback()                                                          #
#                                                                             #
# Action servers:                                                             #
# - complete_task                                                             #
# Service clients:                                                            #
# - register_controller                                                       #
# - update_task                                                               #
###############################################################################

# Imports
import rclpy
from rclpy.node import Node
from rclpy.action import ActionServer #? orignal has broader import statement
from rclpy.action import GoalResponse, CancelResponse
from sec_interfaces.action import CompleteTask
from sec_interfaces.srv import RegisterController
from sec_interfaces.srv import UpdateTask

# Constants
RETRY_INTERVAL = 1.0 # Retry interval in seconds

###############################################################################
# Controller Class Definition                                                 #
###############################################################################
class Controller(Node):
   # Constructor
   def __init__(self, name, tick_rate_ms): #? does tick_rate need an ': int'?
      # Initialize basic data members
      super().__init__(name)
      self.controller_name = name
      self.timer_tick_rate = tick_rate_ms

      # Create the complete_task action server
      self.complete_task = ActionServer(
         self,
         CompleteTask,
         f"{name}/complete_task",
         goal_callback = self.handle_goal,
         cancel_callback = self.handle_cancel,
         handle_accepted_callback = self.handle_accepted
      )

      # Create service clients for register_controller and update_task
      self.register_controller = self.create_client(RegisterController, f"{name}/register_controller")
      self.update_task = self.create_client(UpdateTask, f"{name}/update_task")
      
      # Create a timer
      self.timer = self.create_timer( #? create_timer not in original
         tick_rate_ms / 1000.0,
         self.timer_callback
      )

      # Register the controller with the manager
      self.register_with_manager()




   def handle_goal():
      return
   def handle_cancel():
      return
   def handle_accepted():
      return
   def execute():
      return