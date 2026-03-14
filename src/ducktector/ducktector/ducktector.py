#!/usr/bin/env python3

import numpy as np
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from std_msgs.msg import String
from cv_bridge import CvBridge, CvBridgeError
import cv2
from rclpy.qos import qos_profile_sensor_data
from geometry_msgs.msg import Polygon, Point32

MAX_DUCKS         = 6    # Maximum number of ducks in the arena
MIN_AREA          = 0    # Minimum area of yellow that the algorithm considers important enough to process
MIN_HUE           = 20   # Minimum hue of mask color range
MAX_HUE           = 30   # Maximum hue of mask color range
MIN_SATURATION    = 100  # Mininum saturation of mask color range
MAX_SATURATION    = 255  # Maximum saturation of mask color range
MIN_BRIGHTNESS    = 100  # Minimum brightness of mask color range
MAX_BRIGHTNESS    = 255  # Maximum brightness of mask color range
FOCAL_LENGTH      = 1075 # Perceived focal length. Bigger focal length = object is perceived as further away from the camera

class Ducktector(Node):

    def __init__(self):
        super().__init__('ducktector')

        self.latest_image = None

        self.timer = self.create_timer(
            0.1,  # 10 FPS
            self.process_frame)

        self.bridge = CvBridge()

        self.subscription = self.create_subscription(
            Image,
            '/oak/rgb/image_raw', # Edit this to change which image data topic is being used
            self.image_callback,
            qos_profile_sensor_data
        )

        self.publisher = self.create_publisher(
            Polygon,
            'ducktection',
            10
        )

        self.get_logger().info("ducktector node started.")

    def process_frame(self):
        if self.latest_image is None:
            return

        msg = self.latest_image
        self.latest_image = None

        try:
            # Convert ROS Image message to OpenCV image
            cv_image = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
            cv_image = cv2.GaussianBlur(cv_image, (5,5), 0)

            # Convert OpenCV to HSV image
            hsv_image = cv2.cvtColor(cv_image, cv2.COLOR_BGR2HSV)

            # Define the bounds of what is considered "yellow"
            # first # is hue, second # is saturation, third # is value (or brightness)
            lower_yellow = np.array([MIN_HUE, MIN_SATURATION, MIN_BRIGHTNESS])
            upper_yellow = np.array([MAX_HUE, MAX_SATURATION, MAX_BRIGHTNESS])

            # Create a mask to separate the yellow from the rest of the image
            mask = cv2.inRange(hsv_image, lower_yellow, upper_yellow)
            kernel = np.ones((5,5), np.uint8)
            mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
            
            # Find the centroid of the detected yellow
            M            = cv2.moments(mask)
            duck_points  = Polygon()

            contours, hierarchy = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
            contours = sorted(contours, key=cv2.contourArea, reverse=True)[:MAX_DUCKS]

            for contour in contours:
                area = cv2.contourArea(contour)

                if area > MIN_AREA:
                    M = cv2.moments(contour)

                if M["m00"] != 0:
                    cx = int(M["m10"] / M["m00"])
                    cy = int(M["m01"] / M["m00"])

                    x, y, w, h = cv2.boundingRect(contour)

                    distance = int(FOCAL_LENGTH / w) if w > 0 else -1

                    position_msg   = Point32()
                    position_msg.x = float(cx)
                    position_msg.y = float(cy)
                    position_msg.z = float(distance)

                    duck_points.points.append(position_msg)

                    # Draw centroid
                    #cv2.circle(cv_image, (cx, cy), 8, (0,0,255), -1)

                    # Draw bounding box
                    #cv2.rectangle(cv_image,(x,y),(x+w,y+h),(0,255,0),2)

            if not duck_points.points:
                position_msg   = Point32()
                position_msg.x = -1.0
                position_msg.y = -1.0
                position_msg.z = -1.0
                duck_points.points.append(position_msg)

            # Publish duck position
            self.publisher.publish(duck_points)

            # Optional visualization
            # cv2.imshow("Mask", mask)
            # cv2.imshow("Camera Feed", cv_image)
            # cv2.waitKey(1)

        except CvBridgeError as e:
            self.get_logger().error(f"CV Bridge error: {e}")

    def image_callback(self, msg):
        self.latest_image = msg

def main(args=None):
    rclpy.init(args=args)
    node = Ducktector()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass

    node.destroy_node()
    rclpy.shutdown()
    # cv2.destroyAllWindows()


if __name__ == '__main__':
    main()