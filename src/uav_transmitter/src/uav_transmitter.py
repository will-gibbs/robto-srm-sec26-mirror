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






###############################################################################
###############################################################################
#
# ChatGPT skeleton code for sending the radio signals:
#
###############################################################################
###############################################################################



"""
expresslrs_transmitter.py

Purpose
-------
Provide a simple and extremely clear implementation for transmitting
ExpressLRS-compatible CRSF packets from a Raspberry Pi 5 to an ELRS
transmitter module over UART.

Hardware Setup
--------------
Raspberry Pi 5 UART TX  ->  ELRS module RX
Raspberry Pi 5 UART RX  ->  ELRS module TX (optional)
Grounds must be shared.

Typical UART device on Raspberry Pi:
    /dev/serial0
or
    /dev/ttyAMA0

Typical CRSF baud rate:
    420000
"""

import serial
import time
from typing import List


###############################################################################
# Constants
###############################################################################

CRSF_SYNC_BYTE = 0xC8
CRSF_FRAME_TYPE_RC_CHANNELS_PACKED = 0x16

# ExpressLRS uses 16 RC channels
NUMBER_OF_CHANNELS = 16

# Each CRSF channel value is 11 bits
BITS_PER_CHANNEL = 11

# Valid CRSF channel value range
CRSF_CHANNEL_MIN = 172
CRSF_CHANNEL_MAX = 1811


###############################################################################
# Serial Connection
###############################################################################

def open_serial_connection(device_path: str = "/dev/serial0",
                           baud_rate: int = 420000) -> serial.Serial:
    """
    Open the UART connection to the ExpressLRS transmitter module.

    Parameters
    ----------
    device_path
        Linux device path for the UART interface.

    baud_rate
        CRSF communication speed. ELRS modules typically use 420000.

    Returns
    -------
    serial.Serial
        An open serial connection object.
    """

    serial_connection = serial.Serial(
        port=device_path,
        baudrate=baud_rate,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=0
    )

    return serial_connection


###############################################################################
# CRSF CRC Calculation
###############################################################################

def compute_crsf_crc(data: bytes) -> int:
    """
    Compute the CRSF CRC8 checksum.

    CRSF uses a CRC8 with polynomial 0xD5.

    Parameters
    ----------
    data
        All frame bytes excluding the sync byte.

    Returns
    -------
    int
        Calculated CRC value.
    """

    polynomial = 0xD5
    crc = 0

    for byte in data:
        crc ^= byte

        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ polynomial) & 0xFF
            else:
                crc = (crc << 1) & 0xFF

    return crc


###############################################################################
# Channel Packing
###############################################################################

def pack_rc_channel_values(channel_values: List[int]) -> bytes:
    """
    Pack 16 channel values into the CRSF 11-bit packed format.

    CRSF compresses 16 channels (each 11 bits) into 22 bytes.

    Parameters
    ----------
    channel_values
        List of 16 channel values.

    Returns
    -------
    bytes
        Packed channel byte array.
    """

    if len(channel_values) != NUMBER_OF_CHANNELS:
        raise ValueError("Exactly 16 channel values are required.")

    bit_buffer = 0
    bits_in_buffer = 0
    output_bytes = []

    for value in channel_values:

        if value < CRSF_CHANNEL_MIN or value > CRSF_CHANNEL_MAX:
            raise ValueError("Channel value outside valid CRSF range.")

        bit_buffer |= value << bits_in_buffer
        bits_in_buffer += BITS_PER_CHANNEL

        while bits_in_buffer >= 8:
            output_bytes.append(bit_buffer & 0xFF)
            bit_buffer >>= 8
            bits_in_buffer -= 8

    if bits_in_buffer > 0:
        output_bytes.append(bit_buffer & 0xFF)

    return bytes(output_bytes)


###############################################################################
# CRSF Frame Construction
###############################################################################

def build_crsf_rc_frame(channel_values: List[int]) -> bytes:
    """
    Construct a full CRSF RC channel frame.

    Frame format:

    SYNC
    LENGTH
    TYPE
    PAYLOAD
    CRC

    Parameters
    ----------
    channel_values
        List of 16 channel values.

    Returns
    -------
    bytes
        Complete CRSF frame ready for transmission.
    """

    packed_channels = pack_rc_channel_values(channel_values)

    frame_type = CRSF_FRAME_TYPE_RC_CHANNELS_PACKED

    payload = bytes([frame_type]) + packed_channels

    frame_length = len(payload) + 1  # include CRC

    frame_body = bytes([frame_length]) + payload

    crc = compute_crsf_crc(payload)

    full_frame = bytes([CRSF_SYNC_BYTE]) + frame_body + bytes([crc])

    return full_frame


###############################################################################
# Transmission
###############################################################################

def send_channel_values(serial_connection: serial.Serial,
                        channel_values: List[int]) -> None:
    """
    Send one CRSF RC frame through the serial connection.

    Parameters
    ----------
    serial_connection
        Open UART connection to the ELRS module.

    channel_values
        List of 16 channel values.
    """

    frame = build_crsf_rc_frame(channel_values)

    serial_connection.write(frame)


###############################################################################
# Example Continuous Transmission Loop
###############################################################################

def run_transmitter() -> None:
    """
    Demonstration loop that continuously transmits channel data.

    Real flight control software would replace this with
    actual control inputs.
    """

    serial_connection = open_serial_connection()

    print("ELRS transmitter started")

    neutral_channels = [992] * NUMBER_OF_CHANNELS

    while True:

        send_channel_values(serial_connection, neutral_channels)

        time.sleep(0.004)  # 250 Hz update rate


###############################################################################
# Entry Point
###############################################################################

if __name__ == "__main__":
    run_transmitter()