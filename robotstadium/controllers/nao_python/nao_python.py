# File:         nao_python.py
# Description:  Sample Python controller program for the Robotstadium contest.
#               This can be used as starting example to implement your controller
# Project:      Robotstadium, the online robot soccer competition
# Authors:      Fabien.Rohrer@Cyberbotics.com
# Creation:     March 23, 2009
# Changes:      

from controller import Robot, Accelerometer, Camera, DistanceSensor, \
                       Emitter, Gyro, LED, Motion, Receiver, Motor, \
                       TouchSensor
import struct
import math

# this class is used to create user friendly enumerations
class Enumerate(object):
  def __init__(self, names):
    for number, name in enumerate(names.split()):
      setattr(self, name, number)

# this dictionnary contains the current state of the game
# (received by the supervisor)
class RoboCupGameControlData(dict):

  # Constructor which initializes the structure to receive
  # maxNumPlayers corresponds to MAX_NUM_PLAYERS of RobotCupGameControlData.h
  def __init__(self,maxNumPlayers):
    dict.__init__(self)
    self.maxNumPlayers = maxNumPlayers
    # format defines the format sent to the structure
    format = 'ccccIBBBBBBHI'
    for team in range(0,2):
      format += 'BBH'
      for player in range(0,maxNumPlayers):
        format += 'HH'
    format += 'ff'
    self.gameControlDataStructure = struct.Struct(format)
  
  # import a message received by the C supervisor
  def importData(self,message):
    if len(message) != self.gameControlDataStructure.size:
      print 'Warning: received unknown data packet'
    data = self.gameControlDataStructure.unpack(message)
    counter = 0
    self['header'] = data[counter] + data[counter+1] + \
                     data[counter+2] + data[counter+3]
    counter += 4
    self['version'] = data[counter]
    counter += 1
    self['playersPerTeam'] = data[counter]
    counter += 1
    self['state'] = data[counter]
    counter += 1
    self['firstHalf'] = data[counter]
    counter += 1
    self['kickOffTeam'] = data[counter]
    counter += 1
    self['secondaryState'] = data[counter]
    counter += 1
    self['dropInTeam'] = data[counter]
    counter += 1
    self['dropInTime'] = data[counter]
    counter += 1
    self['secsRemaining'] = data[counter]
    counter += 1
    for teamIndex in range(0,2):
      team = {}
      team['teamNumber'] = data[counter]
      counter += 1
      team['teamColour'] = data[counter]
      counter += 1
      team['score'] = data[counter]
      counter += 1
      for playerIndex in range(0, self.maxNumPlayers):
        player = {}
        player['penality'] = data[counter]
        counter += 1
        player['secsTillUnpenalised'] = data[counter]
        counter += 1
        team['player' + str(playerIndex)] = player
      self['team' + str(teamIndex)] = team
    self['ballXPos'] = data[counter]
    counter += 1
    self['ballYPos'] = data[counter]

# this is the main class
class Nao (Robot):
  
  # enumeration
  PlayerType  = Enumerate('GOALIE PLAYER1 PLAYER2 NOTDEFINED')
  PlayerColor = Enumerate('RED BLUE NOTDEFINED')
  
  # this controller
  player = PlayerType.NOTDEFINED
  color  = PlayerColor.NOTDEFINED
  
  # initial selection
  selectedPlayer = PlayerType.PLAYER1
  
  # this dictionnary will contain information about the game
  roboCupGameControlData = RoboCupGameControlData(4)
  
  # define the motion which is currently playing
  currentlyPlaying = None
  
  # the simulation step [ms]
  timeStep = 40
  
  # the currently pressed key
  key = 0
  
  # Constructor: initialize some stuff
  def __init__(self):
    Robot.__init__(self)
    
    # initialize stuff
    self.findAndEnableDevices()
    self.loadMotionFiles()
    
    # only the selcted robot should display the help message
    if self.amISelected():
      self.printHelp()
    
    # walk forwards until a key is pressed
    self.forwards.setLoop(True)
    self.forwards.play()
    while not self.key:
      self.simulationStep()
      self.key = self.keyboardGetKey()
    
    # stop looping this motion
    self.forwards.setLoop(False)
    
  def run(self):
    if self.key:
      if self.key == ord('1'):
        self.selectPlayer(self.PlayerType.GOALIE)
      elif self.key == ord('2'):
        self.selectPlayer(self.PlayerType.PLAYER1)
      elif self.key == ord('3'):
        self.selectPlayer(self.PlayerType.PLAYER2)
      else:
        self.runCommand(self.key)
    self.simulationStep()
    self.key = self.keyboardGetKey()

  def myColorString(self):
    if self.color == self.PlayerColor.RED:
      return 'RED'
    elif self.color == self.PlayerColor.BLUE:
      return 'BLUE'
    return None

  def findAndEnableDevices(self):
    name = self.getName()
    print name
    if name.find('red') != -1:
      self.color = self.PlayerColor.RED
    if name.find('blue') != -1:
      self.color = self.PlayerColor.BLUE
    if name.find('goal keeper') != -1:
      self.player = self.PlayerType.GOALIE
    if name.find('player 1') != -1:
      self.player = self.PlayerType.PLAYER1
    if name.find('player 2') != -1:
      self.player = self.PlayerType.PLAYER2
    if self.color == self.PlayerColor.NOTDEFINED or \
       self.player == self.PlayerType.NOTDEFINED:
      print 'Error: invalid robot name: ' + name
      while True: self.step(self.timeStep)
    
    # camera
    self.camera = self.getCamera('camera')
    self.camera.enable(4*self.timeStep)
    
    # camera selection (high/low)
    self.cameraSelect = self.getMotor('CameraSelect')
    
    # accelerometer
    self.accelerometer = self.getAccelerometer('accelerometer')
    self.accelerometer.enable(self.timeStep)
    
    # gyro
    self.gyro = self.getGyro('gyro')
    self.gyro.enable(self.timeStep)
    
    # ultrasound sensors
    self.us = []
    usNames = ['USSensor1','USSensor2','USSensor3','USSensor4']
    for i in range(0, len(usNames)):
      self.us.append(self.getDistanceSensor(usNames[i]))
      self.us[i].enable(self.timeStep)
    
    # foot sensors
    self.fsr = [[], []]
    fsrNames = [['LFsr'], ['RFsr']]
    for i in range(0, len(fsrNames)):
      self.fsr[i].append(self.getTouchSensor(fsrNames[i]))
      self.fsr[i].enable(self.timeStep)
    
    # foot bumpers
    self.lfootlbumper = self.getTouchSensor('BumperLFootLeft')
    self.lfootrbumper = self.getTouchSensor('BumperLFootRight')
    self.rfootlbumper = self.getTouchSensor('BumperRFootLeft')
    self.rfootrbumper = self.getTouchSensor('BumperRFootRight')
    self.lfootlbumper.enable(self.timeStep)
    self.lfootrbumper.enable(self.timeStep)
    self.rfootlbumper.enable(self.timeStep)
    self.rfootrbumper.enable(self.timeStep)
    
    # There are 7 controlable LED groups in Webots
    self.leds = []
    self.leds.append(self.getLED('ChestBoard/Led'))
    self.leds.append(self.getLED('RFoot/Led'))
    self.leds.append(self.getLED('LFoot/Led'))
    self.leds.append(self.getLED('Face/Led/Right'))
    self.leds.append(self.getLED('Face/Led/Left'))
    self.leds.append(self.getLED('Ears/Led/Right'))
    self.leds.append(self.getLED('Ears/Led/Left'))
    
    # emitter & receiver
    self.emitter = self.getEmitter('emitter')
    self.receiver = self.getReceiver('receiver')
    self.receiver.enable(self.timeStep)
    
    # for sending 'move' request to Supervisor
    self.superEmitter = self.getEmitter('super_emitter')
    
    # keyboard
    self.keyboardEnable(10*self.timeStep) 

  # load motion files
  def loadMotionFiles(self):
    self.handWave = Motion('../../motions/HandWave.motion')
    self.forwards = Motion('../../motions/Forwards.motion')
    self.backwards = Motion('../../motions/Backwards.motion')
    self.sideStepLeft = Motion('../../motions/SideStepLeft.motion')
    self.sideStepRight = Motion('../../motions/SideStepRight.motion')
    self.turnLeft60 = Motion('../../motions/TurnLeft60.motion')
    self.turnRight60 = Motion('../../motions/TurnRight60.motion')

  def startMotion(self,motion):
    # interrupt current motion
    if self.currentlyPlaying:
      self.currentlyPlaying.stop()
    
    # start new motion
    motion.play()
    self.currentlyPlaying = motion

  def receiveDataPackets(self):
    # if the queue is empty: nothing to print
    if not self.receiver.getQueueLength():
      return

    # keep only the most recent packet
    while self.receiver.getQueueLength() > 1:
      self.receiver.nextPacket()
    
    # import the received packet into the roboCupGameControlData attribute
    packetBytes = self.receiver.getData()
    self.roboCupGameControlData.importData(packetBytes)

  # robocup game control data
  def printGameControlData(self):
    print
    print '----------RoboCupGameControlData----------'
    print self.roboCupGameControlData
    print '----------RoboCupGameControlData----------'

  # the accelerometer axes are oriented as on the real robot
  # however the sign of the returned values may be opposite
  def printAcceleration(self):
    acc = self.accelerometer.getValues()
    print
    print '----------accelerometer----------'
    print 'acceleration: [ x y z ] = ' + str(acc)
    print '----------accelerometer----------'

  # the gyro axes are oriented as on the real robot
  # however the sign of the returned values may be opposite
  def printGyro(self):
    vel = self.gyro.getValues()
    print
    print '----------gyro----------'
    # z value is meaningless due to the orientation of the Gyro
    print 'angular velocity: [ x y ] = ' + str(vel[0:2])
    print '----------gyro----------'

  def printFootSensors(self):
    newtons = 0.0
    fsv = [[],[]] # force sensor values
    
    fsv[0].append(self.fsr[0][0].getValues())
    fsv[1].append(self.fsr[1][0].getValues())
    
    l = []
    r = []
      
    newtonsLeft = 0
    newtonsRight = 0
    
    # The coefficients were calibrated against the real 
    # robot so as to obtain realistic sensor values.                             
    l.append(fsv[0][2]/3.4 + 1.5*fsv[0][0] + 1.15*fsv[0][1]) # Left Foot Front Left
    l.append(fsv[0][2]/3.4 + 1.5*fsv[0][0] - 1.15*fsv[0][1]) # Left Foot Front Right
    l.append(fsv[0][2]/3.4 - 1.5*fsv[0][0] - 1.15*fsv[0][1]) # Left Foot Rear Right
    l.append(fsv[0][2]/3.4 - 1.5*fsv[0][0] + 1.15*fsv[0][1]) # Left Foot Rear Left
    
    r.append(fsv[1][2]/3.4 + 1.5*fsv[1][0] + 1.15*fsv[1][1]) # Right Foot Front Left
    r.append(fsv[1][2]/3.4 + 1.5*fsv[1][0] - 1.15*fsv[1][1]) # Right Foot Front Right
    r.append(fsv[1][2]/3.4 - 1.5*fsv[1][0] - 1.15*fsv[1][1]) # Right Foot Rear Right
    r.append(fsv[1][2]/3.4 - 1.5*fsv[1][0] + 1.15*fsv[1][1]) # Right Foot Rear Left

    for i in range(0, len(l)):
      l[i] = max(min(l[i], 25), 0)
      r[i] = max(min(r[i], 25), 0)
      newtonsLeft += l[i]
      newtonsRight += r[i]
    
    print
    print '----------foot sensors----------'
    print '   left       right'
    print '+--------+ +--------+'
    print '|'  + str(round(l[0],1)) + \
          '  ' + str(round(l[1],1)) + \
          '| |'+ str(round(r[0],1)) + \
          '  ' + str(round(r[1],1)) + \
          '|  front'
    print '|        | |        |'
    print '|'  + str(round(l[3],1)) + \
          '  ' + str(round(l[2],1)) + \
          '| |'+ str(round(r[3],1)) + \
          '  ' + str(round(r[2],1)) + \
          '|  back'
    print '+--------+ +--------+'
    print 'total: ' + str(newtonsLeft + newtonsRight) + \
          ' Newtons, ' + str((newtonsLeft + newtonsRight)/9.81) + ' kilograms'
    print '----------foot sensors----------'

  def printFootBumpers(self):
    ll = self.lfootlbumper.getValue()
    lr = self.lfootrbumper.getValue()
    rl = self.rfootlbumper.getValue()
    rr = self.rfootrbumper.getValue()
    print
    print '----------foot bumpers----------'
    print '   left       right'
    print '+--------+ +--------+'
    print '|'  + str(ll) + '  ' + str(lr) + '| |'+ str(rl) + '  ' + str(rr) + '|'
    print '|        | |        |'
    print '|        | |        |'
    print '+--------+ +--------+'
    print '----------foot bumpers----------'

  def printUltrasoundSensors(self):
    dist = []
    for i in range(0, len(self.us)):
      dist.append(self.us[i].getValue())

    print
    print '-----ultrasound sensors-----'
    print 'top:   left: ' + str(dist[2]) + \
          ' m, right ' + str(dist[0]) + ' m'
    print 'bottom left: ' + str(dist[3]) + \
          ' m, right ' + str(dist[1]) + ' m'
    print '-----ultrasound sensors-----'

  def printCameraImage(self):
    scaled = 2 # defines by which factor the image is subsampled
    width = self.camera.getWidth()
    height = self.camera.getHeight()
    
    # read rgb pixel values from the camera
    image = self.camera.getImageArray()
    
    print
    print '----------camera image (grey levels)---------'
    print 'original resolution: ' + str(width) + ' x ' + \
           str(height) + ', scaled to ' + str(width/scaled) + \
           ' x ' + str(height/scaled)
    for x in range(0, width/scaled):
      line = ''
      for y in range(0, height/scaled):
        grey = image[x*scaled][y*scaled][0] + \
               image[x*scaled][y*scaled][1] + \
               image[x*scaled][y*scaled][2]
        grey = grey / 3
        grey = grey * 9 / 255 # between 0 and  instead of 0 and 255
        line = line + str(int(grey))
      print line
    print '----------camera image----------'

  def setAllLedsColor(self,rgb):
    # these leds take RGB values
    for i in range(0, len(self.leds)):
      self.leds[i].set(rgb)

    # ear leds are single color (blue)
    # and take values between 0 - 255
    self.leds[5].set(rgb & 0xFF)
    self.leds[6].set(rgb & 0xFF)

  # call this function to set the robot position
  # [tx ty tz]: the robot's new position, alpha: the robot's heading direction
  # For training only: this feature will be disabled during Robotstadium contest rounds
  def sendMoveRobotRequest(self,tx,ty,tz,alpha):
    request = 'move robot '
    request += self.myColorString() + ' '
    request += str(self.player) + ' '
    request += str(tx) + ' '
    request += str(ty) + ' '
    request += str(tz) + ' '
    request += str(alpha)
    request += '\0' # explicit termination of the string because of the supervisor is written in C
    self.superEmitter.send(request);

  # call this function to set the ball position
  # [tx ty tz]: the ball's new position
  # For training only: this feature will be disabled during Robotstadium contest rounds
  def sendMoveBallRequest(self,tx,ty,tz):
    request = 'move ball '
    request += str(tx) + ' '
    request += str(ty) + ' '
    request += str(tz)
    request += '\0' # explicit termination of the string because of the supervisor is written in C
    self.superEmitter.send(request);

  def printHelp(self):
    print
    print '----------nao_python.py (demo)----------'
    print 'Use the keyboard to control the robots (one at a time)'
    print '(The 3D window need to be focused)'
    print '[1][2][3]: select a player (it will wave hand !)'
    print '[Up][Down]: move one step forward/backwards'
    print '[<-][->]: side step left/right'
    print '[Shift] + [<-][->]: turn left/right'
    print '[U]: print ultrasound sensors'
    print '[A]: print accelerometers'
    print '[G]: print gyros'
    print '[F]: print foot sensors'
    print '[B]: print foot bumpers'
    print '[D]: print RoboCupGameControlData'
    print '[I]: print scaled camera image'
    print '[Home][End]: camera selection (high/low)'
    print '[7][8][9]: change all leds RGB color'
    print '[0]: turn all leds off'
    print '[X]: teleport selected robot'
    print '[Y]: teleport the ball'
    print '[H]: print this help message'
    print '----------nao_python.py (demo)----------'
  
  def terminate(self):
    #add you cleanup code here: write results, close files, free memory, etc.
    exit(0)
    
  def simulationStep(self):
    if self.step(self.timeStep) == -1:
      self.terminate()
    
    self.receiveDataPackets()

  def amISelected(self):
    return self.player == self.selectedPlayer

  def runCommand(self, key):

    if not self.amISelected():
      return
    
    if key == Robot.KEYBOARD_LEFT:
      self.startMotion(self.sideStepLeft)
    elif key == Robot.KEYBOARD_RIGHT:
      self.startMotion(self.sideStepRight)
    elif key == Robot.KEYBOARD_UP:
      self.startMotion(self.forwards)
    elif key == Robot.KEYBOARD_DOWN:
      self.startMotion(self.backwards)
    elif key == Robot.KEYBOARD_LEFT | Robot.KEYBOARD_SHIFT:
      self.startMotion(self.turnLeft60)
    elif key == Robot.KEYBOARD_RIGHT| Robot.KEYBOARD_SHIFT:
      self.startMotion(self.turnRight60)
    elif key == ord('D'):
      self.printGameControlData()
    elif key == ord('A'):
      self.printAcceleration()
    elif key == ord('G'):
      self.printGyro()
    elif key == ord('F'):
      self.printFootSensors()
    elif key == ord('B'):
      self.printFootBumpers()
    elif key == ord('U'):
      self.printUltrasoundSensors()
    elif key == ord('I'):
      self.printCameraImage()
    elif key == Robot.KEYBOARD_HOME:
      self.cameraSelect.setPosition(0)
    elif key == Robot.KEYBOARD_END:
      self.cameraSelect.setPosition(1)
    elif key == ord('7'):
      self.setAllLedsColor(0xff0000) # red
    elif key == ord('8'):
      self.setAllLedsColor(0x00ff00) # green
    elif key == ord('9'):
      self.setAllLedsColor(0x0000ff) # blue
    elif key == ord('0'):
      self.setAllLedsColor(0x000000) # off
    elif key == ord('X'):
      self.sendMoveRobotRequest(0.0, 1.0, 0.0, 1.5708)
    elif key == ord('Y'):
      self.sendMoveBallRequest(0.0, 1.0, 0.0)
    elif key == ord('H'):
      self.printHelp()
    
  def selectPlayer(self,p):
    self.selectedPlayer = p
    if self.amISelected():
      print 'selected: ' + self.getName()
      self.handWave.play()

# instanciate a Robot instance and enter in an infinit loop
controller = Nao()
while True:
  controller.run()
