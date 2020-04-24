
import serial
import rospy
from geometry_msgs.msg import Twist

# open serial port
port = '/dev/ttyUSB1'
ard = serial.Serial(port, 9600, timeout=5)

# create new node
rospy.init_node('serial_cmd_vel', anonymous=True)
velocity_publisher = rospy.Publisher('cmd_vel', Twist, queue_size=10)
vel_msg = Twist()

def updateAndPub():
    SPEED_SCALER = 5
    while(True):
         msg = ard.readline()[:-2]
         msgList1 = msg.split(' ')

         # in case key is rotated
         if(len(msgList1) == 1):
              print("New key on RX side: " + str(msgList1))

         else:
              # extract joystick data
              Xaxis = float(msgList1[1])
              Yaxis = float(msgList1[3])

              # setup message
              vel_msg.linear.x = Xaxis * SPEED_SCALER
              vel_msg.angular.z = Yaxis

              # publish message
              velocity_publisher.publish(vel_msg)

try:
    updateAndPub()
except rospy.ROSInterruptException: pass

ard.close()
