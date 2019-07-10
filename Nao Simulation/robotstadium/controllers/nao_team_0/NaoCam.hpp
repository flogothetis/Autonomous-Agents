#ifndef NAO_CAM_HPP
#define NAO_CAM_HPP

//-----------------------------------------------------------------------------
//  File:         NaoCam C++ class (to be used in a Webots controllers)
//  Description:  Dual Nao Camera with top/bottom selection and simple blob localization capability
//  Project:      Robotstadium, the online robot soccer competition
//  Author:       Yvan Bourquin - www.cyberbotics.com
//  Date:         May 5, 2008
//  Changes:      August 23, 2012: Adapted to dual camera system
//-----------------------------------------------------------------------------

namespace webots {
  class Robot;
  class Camera;
}

using namespace webots;

class NaoCam {
public:
  // value when object direction or elevation is unknown
  static const double UNKNOWN;

  // target goal colors
  enum { SKY_BLUE, YELLOW, UNKNOWN_COLOR };

  // constructor
  NaoCam(Robot *robot, int timeStep);

  // set target color
  void setGoalColor(int goalColor) { this->goalColor = goalColor; }

  // find ball and goal in most recent image
  void processImage();

  // direction and elevation angles are indicated in radians
  // with respect to the camera focal (or normal) line
  // therefore a direction angle will not exceed +/- half the fieldOfView
  // a positive direction is towards the right of the camera image
  // a positive elevation is towards the top of the camera image
  double getBallDirectionAngle() const { return ballDirectionAngle; }
  double getBallElevationAngle() const { return ballElevationAngle; }
  double getGoalDirectionAngle() const { return goalDirectionAngle; }

  // selection top or bottom camera (the top camera is selected initially)
  void selectTop();
  void selectBottom();
  double getOffsetAngle() const;
  bool isTopSelected() const { return topSelected; }

  // 40 degrees angle between top and bottom camera axes
  static const double OFFSET_ANGLE;

protected:
  void findColorBlob(const double ref[3], double tolerance, double &direction, double &elevation);
  const unsigned char *getImage();

private:
  bool topSelected;
  Camera *topCamera;
  Camera *bottomCamera;
  int timeStep;
  const unsigned char *image;
  double fov;  // field of view
  int width;   // pixel width
  int height;  // pixel height
  int goalColor;
  double ballDirectionAngle;
  double ballElevationAngle;
  double goalDirectionAngle;
};

#endif
