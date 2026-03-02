#!/usr/bin/env python3

import numpy as np
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from std_msgs.msg import String
from cv_bridge import CvBridge, CvBridgeError
import cv2
from rclpy.qos import qos_profile_sensor_data

MIN_HUE        = 20  # Minimum hue of mask color range
MAX_HUE        = 30  # Maximum hue of mask color range
MIN_SATURATION = 100 # Mininum saturation of mask color range
MAX_SATURATION = 255 # Maximum saturation of mask color range
MIN_BRIGHTNESS = 100 # Minimum brightness of mask color range
MAX_BRIGHTNESS = 255 # Maximum brightness of mask color range

class Ducktector(Node):

    def __init__(self):
        super().__init__('ducktector')

        self.bridge = CvBridge()

        self.subscription = self.create_subscription(
            Image,
            '/oak/rgb/image_raw', # Edit this to change which image data topic is being used
            self.image_callback,
            qos_profile_sensor_data
        )

        self.publisher = self.create_publisher(
            String,
            'ducktection',
            10
        )

        self.get_logger().info("ducktector node started.")

    def image_callback(self, msg):
        try:
            # Convert ROS Image message to OpenCV image
            cv_image = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')

            # Convert OpenCV to HSV image
            hsv_image = cv2.cvtColor(cv_image, cv2.COLOR_BGR2HSV)
    
            # Define the bounds of what is considered "yellow"
            # first # is hue, second # is saturation, third # is value (or brightness)
            lower_yellow = np.array([MIN_HUE, MIN_SATURATION, MIN_BRIGHTNESS])
            upper_yellow = np.array([MAX_HUE, MAX_SATURATION, MAX_BRIGHTNESS])

            # Create a mask to separate the yellow from the rest of the image
            mask = cv2.inRange(hsv_image, lower_yellow, upper_yellow)
            
            # Find the centroid of the detected yellow
            M = cv2.moments(mask)

            height, width = mask.shape

            position_msg = String()

            if M["m00"] > 0:
                cx = int(M["m10"] / M["m00"])

                # Determine which side the duck is on
                if cx < width / 3:
                    position = "LEFT"
                elif cx < 2 * width / 3:
                    position = "CENTER"
                else:
                    position = "RIGHT"

                position_msg.data = position

                # Optional: draw centroid
                cv2.circle(cv_image, (cx, height // 2), 10, (0, 0, 255), -1)

            else:
                position_msg.data = "NO DUCK"

            # Publish duck position
            self.publisher.publish(position_msg)

            # Optional visualization
            cv2.imshow("Mask", mask)
            cv2.imshow("Camera Feed", cv_image)
            cv2.waitKey(1)
            
            # Apply the mask to the original image
            #detected_output = cv2.bitwise_and(cv_image, cv_image, mask=mask)

            # Display the new image
            #cv2.imshow("Camera Feed", detected_output)
            #cv2.waitKey(1)

        except CvBridgeError as e:
            self.get_logger().error(f"CV Bridge error: {e}")


def main(args=None):
    rclpy.init(args=args)
    node = Ducktector()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass

    node.destroy_node()
    rclpy.shutdown()
    cv2.destroyAllWindows()


if __name__ == '__main__':
    main()