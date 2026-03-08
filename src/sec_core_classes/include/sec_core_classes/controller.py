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
import threading
import rclpy
from rclpy.node import Node
from rclpy.action import ActionServer
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
   def __init__(self, name: str, tick_rate_ms: int):
      # Initialize basic data members
      super().__init__(name)
      self.controller_name = name
      self.timer_tick_rate = tick_rate_ms

      # Create the complete_task action server
      self.complete_task = ActionServer(
         self,
         CompleteTask,
         f"{name}/complete_task",
         goal_callback=self.handle_goal,
         cancel_callback=self.handle_cancel,
         handle_accepted_callback=self.handle_accepted
      )

      # Create service clients for register_controller and update_task
      self.register_controller = self.create_client(
         RegisterController,
         f"{name}/register_controller"
      )
      self.update_task = self.create_client(UpdateTask, f"{name}/update_task")
      
      # Create a timer
      self.timer = self.create_timer(
         tick_rate_ms / 1000.0,
         self.timer_callback
      )

      # Register the controller with the manager
      self.register_with_manager()

   # Manage receiving a controller goal
   def handle_goal(self, goal_request):
      self.get_logger().info(
         f"Received goal with begin task value of {goal_request.begin_task}"
      )
      return GoalResponse.ACCEPT

   # Mange a request to cancel a goal
   def handle_cancel(self, goal_handle):
      self.get_logger().info("Received cancel goal request.")
      return CancelResponse.ACCEPT
   
   # Determines whether the goal is acceptable
   def handle_accepted(self, goal_handle):
      self.get_logger().info("Accepted a goal.")
      thread = threading.Thread(
         target=self.execute,
         args=(goal_handle,),
         daemon=True
      )
      thread.start()
   
   # Execute the goal
   def execute(self, goal_handle):
      self.get_logger().info("Executing goal")
      goal = goal_handle.request
      return
   
   # Register the controller with the manager by creating a RegisterController
   # service client
   def register_with_manager(self):
      request = RegisterController.Request()
      request.controller_action_name = self.controller_name
      if not self.register_controller.wait_for_service(timeout_sec=RETRY_INTERVAL):
         self.get_logger().warning(
            f"RegisterController service not available for {self.controller_name}"
         )
         return
      
      future = self.register_controller.call_async(request)
      future.add_done_callback(self.register_callback)

      def register_callback(self, future):
         result = future.result()
         if result.registration_status_code == RegisterController.Response.OK:
            self.get_logger().info(
               f"The {self.controller_name} controller registered successfully."
            )

   # Loops to perform a task
   def timer_callback(self):
      pass