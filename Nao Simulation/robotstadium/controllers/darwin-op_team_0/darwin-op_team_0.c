#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/led.h>
#include <math.h>
#include <assert.h>

#define TIME_STEP 40
#define NMOTORS   20

static const char *motor_names[NMOTORS] = {
  "ShoulderR" /*ID1 */, "ShoulderL" /*ID2 */, "ArmUpperR" /*ID3 */, "ArmUpperL" /*ID4 */,
  "ArmLowerR" /*ID5 */, "ArmLowerL" /*ID6 */, "PelvYR"    /*ID7 */, "PelvYL"    /*ID8 */,
  "PelvR"     /*ID9 */, "PelvL"     /*ID10*/, "LegUpperR" /*ID11*/, "LegUpperL" /*ID12*/,
  "LegLowerR" /*ID13*/, "LegLowerL" /*ID14*/, "AnkleR"    /*ID15*/, "AnkleL"    /*ID16*/,
  "FootR"     /*ID17*/, "FootL"     /*ID18*/, "Neck"      /*ID19*/, "Head"      /*ID20*/
};

static double amplitudes[6] = {
  0.2, -0.2, 0.5, -0.5, 0.5, -0.5
};

static double offsets[6] = {
  2.5, -2.5, -0.5, 0.5, -1.0, 1.0
};

static double min_motor_positions[NMOTORS];
static double max_motor_positions[NMOTORS];

static double frequency = 1.0;

static double clamp(double value, double min, double max) {
  if (min > max) {
    assert(0);
    return value;
  }
  return value < min ? min : value > max ? max : value;
}

int main(int argc, char **argv)
{
  wb_robot_init();
  
  int i;
  WbDeviceTag motors[NMOTORS];
  WbDeviceTag head_led;
  
  for (i=0; i<NMOTORS; i++) {
    motors[i] = wb_robot_get_device(motor_names[i]);
    min_motor_positions[i] = wb_motor_get_min_position(motors[i]);
    max_motor_positions[i] = wb_motor_get_max_position(motors[i]);
  }
  head_led = wb_robot_get_device("HeadLed");
  wb_led_set(head_led, 0x40C040);
  
  do {
    double t = wb_robot_get_time();
    for (i=0; i<6; i++) {
      double targetPosition = amplitudes[i]*sin(frequency*t) + offsets[i];
      targetPosition = clamp(targetPosition, min_motor_positions[i], max_motor_positions[i]);
      wb_motor_set_position(motors[i], targetPosition);
    }
  } while (wb_robot_step(TIME_STEP) != -1);
  
  wb_robot_cleanup();
  
  return 0;
}

