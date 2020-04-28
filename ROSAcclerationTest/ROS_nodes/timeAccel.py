import serial
import rospy
from geometry_msgs.msg import Twist
from sensor_msgs.msg import LaserScan
import time

# create global control variables
global lastVel
lastVel = 0
global inCollision
inCollision = False
global runTime
runTime = 0

# define threshold for scan
SCAN_TRESH = 0.17

# create cmd_vel callback
def cmdVelCallback(data):
     global lastVel
     global runTime
     linearVel = data.linear.x
     if(linearVel > 0 and lastVel == 0):
          isMoving = True
          runTime = time.time()

     lastVel = linearVel

# create scan callback
def scanCallback(data):
     global inCollision, runTime
     totalTime = 0
     for elem in data.ranges:
          if(elem <= SCAN_TRESH and not inCollision):
                inCollision = True
                totalTime = time.time() - runTime
                print("Total Time: " + str(totalTime))

     numOver = 0
     for elem in data.ranges:
          if(elem <= SCAN_TRESH):
               numOver += 1
     if(numOver == 0):
          inCollision = False

# create new ROS node
rospy.init_node('serial_cmd_vel', anonymous=True)
rospy.Subscriber("cmd_vel", Twist, cmdVelCallback)
rospy.Subscriber("scan", LaserScan, scanCallback)
rospy.spin()
