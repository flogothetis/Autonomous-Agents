#include "Player.hpp"
#include "NaoCam.hpp"
#include "InfoMessage.hpp"
#include "../nao_soccer_supervisor/RoboCupGameControlData.h"
#include <webots/Accelerometer.hpp>
#include <webots/Gyro.hpp>
#include <webots/InertialUnit.hpp>
#include <webots/Motor.hpp>
#include <webots/LED.hpp>
#include <webots/TouchSensor.hpp>
#include <webots/Emitter.hpp>
#include <webots/Receiver.hpp>
#include <webots/GPS.hpp>
#include <webots/DistanceSensor.hpp>
#include <webots/Motor.hpp>
#include <webots/utils/Motion.hpp>
#include <string>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cassert>
#include <iostream>

using namespace std;
using namespace webots;

double Player::clamp(double value, double min, double max) {
  if (min > max) {
    assert(0);
    return value;
  }
  return value < min ? min : value > max ? max : value;
}


const int Player::SIMULATION_STEP = 40;  // milliseconds
const int CAMERA_STEP = 160;  // camera refresh rate in milliseconds
const double CAMERA_OFFSET_ANGLE = 0.6981;  // 40 degrees between cameras axes

bool Player::isBlue() const {
  return gameControlData->teams[TEAM_BLUE].teamNumber == teamID;
}

bool Player::isRed() const {
  return gameControlData->teams[TEAM_RED].teamNumber == teamID;
}

Player::Player(int playerID, int teamID) {
 
  // need space to store incoming data
  gameControlData = new RoboCupGameControlData;
  gameControlData->state = STATE_INITIAL;

  this->playerID = playerID;
  this->teamID = teamID;

  // initialize accelerometer
  accelerometer = getAccelerometer("accelerometer");
  //accelerometer->enable(SIMULATION_STEP);   // uncomment only if needed !

  // initialize gyro
  gyro = getGyro("gyro");
  //gyro.enable(SIMULATION_STEP);   // uncomment only if needed !
  
  // initialize inertial unit
  inertialUnit = getInertialUnit("inertial unit");
  inertialUnit->enable(SIMULATION_STEP);

  // get "HeadYaw" and "HeadPitch" motors and enable position feedback
  headYaw = getMotor("HeadYaw");
  mMinHeadYawPosition = headYaw->getMinPosition();
  mMaxHeadYawPosition = headYaw->getMaxPosition();
  headYaw->enablePosition(SIMULATION_STEP);
  headPitch = getMotor("HeadPitch");
  headPitch->enablePosition(SIMULATION_STEP);
  mMinHeadPitchPosition = headPitch->getMinPosition();
  mMaxHeadPitchPosition = headPitch->getMaxPosition();

  // get all LEDs
  chestLed = getLED("ChestBoard/Led");
  rightEyeLed = getLED("Face/Led/Right");
  leftEyeLed = getLED("Face/Led/Left");
  rightEarLed = getLED("Ears/Led/Right");
  leftEarLed = getLED("Ears/Led/Left");
  rightFootLed = getLED("RFoot/Led");
  leftFootLed = getLED("LFoot/Led");

  // make eyes shine blue
  rightEyeLed->set(0x2222ff);
  leftEyeLed->set(0x2222ff);

  // create camera
  camera = new NaoCam(this, CAMERA_STEP);

  // foot sole touch sensors
  const string TOUCH_SENSOR_NAMES[NUM_TOUCH_SENSORS] = {
    "RFsr", "LFsr"
  };

  for (int i = 0; i < NUM_TOUCH_SENSORS; i++) {
    fsr[i] = getTouchSensor(TOUCH_SENSOR_NAMES[i]);
    //fsr[i]->enable(SIMULATION_STEP);  // uncomment only if needed !
  }

  // emitter/receiver devices that can be used for inter-robot communication
  // and for receiving RobotCupGameControleData
  emitter = getEmitter("emitter");
  receiver = getReceiver("receiver");
  receiver->enable(SIMULATION_STEP);

  // for sending 'move' request to Supervisor
  superEmitter = getEmitter("super_emitter");

  // useful to know the position of the robot
  // the real Nao does not have a GPS, this is for testing only
  // therefore the GPS info will be blurred during Robotstadium contest matches
  gps = getGPS("gps");
  //gps->enable(SIMULATION_STEP);  // uncomment only if needed !

  // initialize ultrasound sensors
  topLeftUltrasound = getDistanceSensor("USSensor3");
  topRightUltrasound = getDistanceSensor("USSensor1");
  bottomLeftUltrasound = getDistanceSensor("USSensor4");
  bottomRightUltrasound = getDistanceSensor("USSensor2");
  //topLeftUltrasound->enable(SIMULATION_STEP);  // uncomment only if needed !
  //topRightUltrasound->enable(SIMULATION_STEP);  // uncomment only if needed !
  //bottomLeftUltrasound->enable(SIMULATION_STEP);  // uncomment only if needed !
  //bottomRightUltrasound->enable(SIMULATION_STEP);  // uncomment only if needed !

  // load motion
  standUpFromFrontMotion = new Motion("../../motions/StandUpFromFront.motion");
}

Player::~Player() {
  delete standUpFromFrontMotion;
  delete gameControlData;
}

// play until the specified motion is over
void Player::playMotion(Motion *motion) {

  // don't move if the game is not playing
  if (gameControlData->state != STATE_PLAYING) {
    return;
  }

  // play to the end
  motion->play();
  do {
    runStep();
  }
  while (! motion->isOver());
}

// use inertial unit to detect fall
void Player::getUpIfNecessary() {
  const double *rpy = inertialUnit->getRollPitchYaw();
  if (fabs(rpy[0]) > 1.0 || fabs(rpy[1]) > 1.0)
    playMotion(standUpFromFrontMotion);
}

// move head from left to right and from right to left
// until the ball is sighted or the scan motion is over
void Player::headScan() {
  const int STEPS = 30;
  const double HEAD_YAW_MAX = 2.0;

  headPitch->setPosition(0.0);  // horizontal head
  camera->selectTop();  // use top camera

  // left to right using TOP camera
  for (int i = 0; i < STEPS; i++) {
    double yawAngle = ((double)i / (STEPS - 1) * 2.0 - 1.0) * HEAD_YAW_MAX;
    headYaw->setPosition(clamp(yawAngle, mMinHeadYawPosition, mMaxHeadYawPosition));
    step(SIMULATION_STEP);
    camera->processImage();
    if (camera->getBallDirectionAngle() != NaoCam::UNKNOWN)
      return;
  }

  camera->selectBottom();  // use bottom camera

  // right to left using BOTTOM camera
  for (int i = STEPS - 1; i >= 0; i--) {
    double yawAngle = ((double)i / (STEPS - 1) * 2.0 - 1.0) * HEAD_YAW_MAX;
    headYaw->setPosition(clamp(yawAngle, mMinHeadYawPosition, mMaxHeadYawPosition));
    step(SIMULATION_STEP);
    camera->processImage();
    if (camera->getBallDirectionAngle() != NaoCam::UNKNOWN)
      return;
  }

  // ball was not found: restore head straight position
  headYaw->setPosition(0.0);
}

double Player::getBallDirection() const {
  if (camera->getBallDirectionAngle() == NaoCam::UNKNOWN)
    return NaoCam::UNKNOWN;
  else
    return camera->getBallDirectionAngle() - headYaw->getPosition();
}

// compute floor distance between robot (feet) and ball
// 0.51 -> approx robot camera base height with respect to ground in a standard posture of the robot
// 0.043 -> ball radius
double Player::getBallDistance() const {
  if (camera->getBallElevationAngle() == NaoCam::UNKNOWN)
    return NaoCam::UNKNOWN;

  double ballElev = camera->getBallElevationAngle() - headPitch->getPosition() - camera->getOffsetAngle();
  return (0.51 - 0.043) / tan(-ballElev);
}

// turn head towards ball if ball position is known
void Player::trackBall() {
  const double P = 0.7;
  double ballDirection = camera->getBallDirectionAngle();
  double ballElevation = camera->getBallElevationAngle();

  if (ballDirection == NaoCam::UNKNOWN)
    return;

  // compute target head pitch
  double pitch = headPitch->getPosition() - ballElevation * P;

  if (pitch < -0.4 && ! camera->isTopSelected()) { // need to switch to TOP camera ?
    //cout << "switched to TOP camera\n";
    camera->selectTop();
    pitch += NaoCam::OFFSET_ANGLE;  // move head down 40 degrees
    headPitch->setPosition(clamp(pitch, mMinHeadPitchPosition, mMaxHeadPitchPosition));
    sleepSteps(8);  // allow some time to move head
    camera->processImage();
  }
  else if (pitch > 0.5 && camera->isTopSelected()) { // need to switch to BOTTOM camera ?
    //System.out.println("switched to BOTTOM camera");
    camera->selectBottom();
    pitch -= NaoCam::OFFSET_ANGLE;  // move head up 40 degrees
    headPitch->setPosition(clamp(pitch, mMinHeadPitchPosition, mMaxHeadPitchPosition));
    sleepSteps(8);  // allow some time to move head
    camera->processImage();
  }

  headPitch->setPosition(clamp(pitch, mMinHeadPitchPosition, mMaxHeadPitchPosition));
  double yaw = headYaw->getPosition() - ballDirection * P;
  headYaw->setPosition(clamp(yaw, mMinHeadYawPosition, mMaxHeadYawPosition));
}

void Player::updateGameControl() {

  // choose goal color according to my team's color
  // and display team color on left foot LED
  if (isRed()) {
    camera->setGoalColor(NaoCam::SKY_BLUE);
    leftFootLed->set(0xff2222);
  }
  else {
    camera->setGoalColor(NaoCam::YELLOW);
    leftFootLed->set(0x2222ff);
  }
  
  // update chest button color according to game state
  switch (gameControlData->state) {
    case STATE_INITIAL:
    case STATE_FINISHED:
      chestLed->set(0x000000);  // off
      break;
    case STATE_READY:
      chestLed->set(0x2222ff);  // blue
      break;
    case STATE_SET:
      chestLed->set(0xffff22);  // yellow
      break;
    case STATE_PLAYING:
      chestLed->set(0x22ff22);  // green
      break;
  }
}

// information that describes a team
void Player::printTeamInfo(const struct TeamInfo *team) {
  cout << "  teamNumber: " << (int)team->teamNumber << endl;
  cout << "  teamColour: " << (int)team->teamColour << endl;
  cout << "  score: " << (int)team->score << endl;
}

// robocup game control date
void Player::printGameControlData(const struct RoboCupGameControlData *gcd) {
  cout << "----------RoboCupGameControlData----------\n";
  cout << "playersPerTeam: " << (int)gcd->playersPerTeam << endl;
  cout << "state: " << (int)gcd->state << endl;
  cout << "firstHalf: " << (int)gcd->firstHalf << endl;
  cout << "kickOffTeam: " << (int)gcd->kickOffTeam << endl;
  cout << "secsRemaining: " << (int)gcd->secsRemaining << endl;
  cout << "teams[TEAM_BLUE]:\n";
  printTeamInfo(&gcd->teams[TEAM_BLUE]);
  cout << "teams[TEAM_RED]:\n";
  printTeamInfo(&gcd->teams[TEAM_RED]);
  // For training only: this will be 0.0 during Robotstadium contest rounds:
  cout << "ballXPos: " << gcd->ballXPos << endl;
  cout << "ballZPos: " << gcd->ballZPos << endl;
  cout << "----------RoboCupGameControlData----------\n";
}

void Player::readIncomingMessages() {
  while (receiver->getQueueLength() > 0) {
    const void *data = receiver->getData();

    // important: verify packet header type
    if (memcmp(data, GAMECONTROLLER_STRUCT_HEADER, sizeof(GAMECONTROLLER_STRUCT_HEADER) - 1) == 0) {
      memcpy(gameControlData, data, sizeof(RoboCupGameControlData));
      //printGameControlData(gameControlData);  // uncomment to debug
      updateGameControl();
    }
    else if (memcmp(data, INFO_MESSAGE_STRUCT_HEADER, sizeof(INFO_MESSAGE_STRUCT_HEADER) - 1) == 0) {
      struct InfoMessage info;
      memcpy(&info, data, sizeof(InfoMessage));
      cerr << "player " << info.playerID << " is " << info.distanceToBall << " meters from the ball" << endl;
    }
    // unidentified messages will be silently ignored, otherwise: uncomment these two lines:
    // else
    //   cerr << "readIncomingMessages(): received unexpected message of " << receiver->getDataSize() << " bytes" << endl;

    receiver->nextPacket();
  }
}

// example on how to send an info message to teammates
// these messages are not sent to the Supervisor
void Player::sendInfoMessage() {
  InfoMessage info;
  memcpy(info.header, INFO_MESSAGE_STRUCT_HEADER, sizeof(INFO_MESSAGE_STRUCT_HEADER) - 1);
  info.distanceToBall = getBallDistance();
  // info.blalba = your stuff ...
  emitter->send(&info, sizeof(InfoMessage));
}

// move the robot to a specified position (via a message sent to the Supervisor)
// [tx ty tz]: the new robot position, alpha: the robot's heading direction
// For debugging only: this is disabled during the contest rounds
void Player::sendMoveRobotMessage(double tx, double ty, double tz, double alpha) {
  char buf[256];
  sprintf(buf, "move robot %d %d %f %f %f %f", playerID, teamID, tx, ty, tz, alpha);
  superEmitter->send(buf, strlen(buf) + 1);
}

// move the ball to a specified position (via a message sent to the Supervisor)
// [tx ty tz]: the new ball position
// For debugging only: this is disabled during the contest rounds
void Player::sendMoveBallMessage(double tx, double ty, double tz) {
  char buf[256];
  sprintf(buf, "move ball %f %f %f", tx, ty, tz);
  superEmitter->send(buf, strlen(buf) + 1);
}

// overidden method of the Robot baseclass
// we need to read incoming messages at every step
int Player::step(int ms) {
  readIncomingMessages();
  return Robot::step(ms);
}

void Player::runStep()  {
  trackBall();
  step(SIMULATION_STEP);
  camera->processImage();
}

void Player::sleepSteps(int steps) {
  for (int i = 0; i < steps; i++)
    step(SIMULATION_STEP);
}
