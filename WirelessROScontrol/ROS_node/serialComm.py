
import serial
import rospy
from geometry_msgs.msg import Twist

# open serial port
port = '/dev/ttyUSB1'
ard = serial.Serial(port, 9600, timeout=5)

# create new publisher node
rospy.init_node('serial_cmd_vel', anonymous=True)
velocity_publisher = rospy.Publisher('cmd_vel', Twist, queue_size=10)

# create empty Twist message
vel_msg = Twist()

def updateAndPub():
    """
    This function extracts the user input from the recieved
    data from the serial interface, scales the velocities, and
    constructs and publishes a Twist message over the \cmd_vel topic.

    params:
        None

    returns:
        None
    """
    SPEED_SCALER = 2
    while(True):

         # remove newline and cariage return characters from
         # serial data
         msg = ard.readline()[:-2]

         # split line by the space character
         msgList1 = msg.split(' ')

         # in case key is rotated
         if(len(msgList1) == 1):
              if(msgList1[0] != ''):
                  print("New key on RX side: " + str(msgList1))

         else:
              # extract joystick data
              Xaxis = float(msgList1[1])
              Yaxis = float(msgList1[3])

              # setup message
              vel_msg.linear.x = Xaxis * SPEED_SCALER
              vel_msg.angular.z = -1*Yaxis

              # publish message
              velocity_publisher.publish(vel_msg)

if(__name__ == "__main__"):
    try:
        updateAndPub()
    except rospy.ROSInterruptException:
        pass
    
    # close the serial port
    ard.close()
