#ifndef PLAYER_HPP
#define PLAYER_HPP

//-----------------------------------------------------------------------------
//  File:              Player class (to be used in a Webots controller)
//  Description:       Base class for FieldPlayer and GoalKeeper
//  Project:           Robotstadium, the online robot soccer competition
//  Author:            Yvan Bourquin - www.cyberbotics.com
//  Last Modification: November 2nd, 2012
//-----------------------------------------------------------------------------

#include <webots/Robot.hpp>

namespace webots {
  class Accelerometer;
  class Gyro;
  class InertialUnit;
  class DistanceSensor;
  class LED;
  class TouchSensor;
  class Emitter;
  class Receiver;
  class GPS;
  class Motion;
  class Motor;
}

class NaoCam;

using namespace webots;
using namespace std;

class Player : public Robot {
public:
  Player(int playerID, int teamID);
  virtual ~Player();
  
  // find out which color to play
  bool isBlue() const;
  bool isRed() const;

  enum { NUM_TOUCH_SENSORS = 2 };

  // pure virtual: effective implementation in derived classes
  virtual void run() = 0;
  
  // overridden method
  virtual int step(int ms);
  
  static double clamp(double value, double min, double max);

protected:
  virtual void runStep();
  void playMotion(Motion *motion);
  void getUpIfNecessary();
  double getBallDirection() const;
  double getBallDistance() const;
  void headScan();

  // global control step (must be a multiple of WorldInfo.basicTimeStep)
  static const int SIMULATION_STEP;

  struct RoboCupGameControlData *gameControlData;
  int playerID, teamID;

  // devices
  NaoCam *camera;
  Motor *headYaw, *headPitch;
  Accelerometer *accelerometer;
  Gyro *gyro;
  InertialUnit *inertialUnit;
  DistanceSensor *topLeftUltrasound, *topRightUltrasound, *bottomLeftUltrasound, *bottomRightUltrasound;
  LED *chestLed, *rightEyeLed, *leftEyeLed, *rightEarLed, *leftEarLed, *rightFootLed, *leftFootLed;
  TouchSensor *fsr[NUM_TOUCH_SENSORS];  // force sensitive resistors
  Emitter *emitter, *superEmitter;
  Receiver *receiver;
  GPS *gps;  // for debugging only ! This device does not exist on the real robot.
  Motion *standUpFromFrontMotion;

private:
  void trackBall();
  void updateGameControl();
  void printTeamInfo(const struct TeamInfo *team);
  void printGameControlData(const struct RoboCupGameControlData *gcd);
  void readIncomingMessages();
  void sendInfoMessage();
  void sendMoveRobotMessage(double tx, double ty, double tz, double alpha);
  void sendMoveBallMessage(double tx, double ty, double tz);
  void sleepSteps(int steps);
  
  double mMinHeadYawPosition;
  double mMaxHeadYawPosition;
  double mMinHeadPitchPosition;
  double mMaxHeadPitchPosition;
};

#endif
