#ifndef AUDIO_FILTER_H
#define AUDIO_FILTER_H

#include <stdint.h>
#include <math.h>


const double kSmallestPositiveFloatValue = 1.175494351e-38;         /* min positive value */
const double kSmallestNegativeFloatValue = -1.175494351e-38;        /* min negative value */
const double kPi = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899;
const double DEFAULT_Q = 0.707;


inline bool checkFloatUnderflow(double& value) {
  bool retValue = false;
  if (value > 0.0 && value < kSmallestPositiveFloatValue) {
    value = 0;
    retValue = true;
  }
  else if (value < 0.0 && value > kSmallestNegativeFloatValue) {
    value = 0;
    retValue = true;
  }
  return retValue;
}


class PeakingFilter {
public:
  PeakingFilter() {}
  ~PeakingFilter() {}

  void reset() {
    resetStates();
  }

  void setSampleRate(double sampleRate) {
    this->sampleRate = sampleRate;
    calculateFilterCoeffs();
  }
  
  void setFrequency(double frequency) {
    this->fc = frequency;
    calculateFilterCoeffs();
  }
  
  void setQFactor(double qFactor) {
    if (qFactor <= 0) qFactor = DEFAULT_Q;
    this->Q = qFactor;
    calculateFilterCoeffs();
  }
  
  void setGainDb(double db) {
    this->db = db;
    calculateFilterCoeffs();
  }
  
  double processAudioSample(double xn, uint8_t ch);
  
private:
  void calculateFilterCoeffs();
  
  void resetCoeffs() {
    cf_a0 = cf_a1 = cf_a2 = cf_b1 = cf_b2 = cf_c0 = cf_d0 = 0.0;
    cf_c0 = 1.0;
  }
  
  void resetStates() {
    for (uint8_t ch = 0; ch < numChannels; ++ch) {
      states[ch][0] = 0.0;
      states[ch][1] = 0.0;
      states[ch][2] = 0.0;
      states[ch][3] = 0.0;
    }
  }

private:
  static const uint8_t numChannels = 2;

  double cf_a0 = 0.0;
  double cf_a1 = 0.0;
  double cf_a2 = 0.0;
  double cf_b1 = 0.0;
  double cf_b2 = 0.0;

  double cf_c0 = 0.0;
  double cf_d0 = 0.0;
  
  double states[numChannels][4] = {
    //x_z1 x_z2 y_z1 y_z2
    { 0.0, 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0, 0.0 }
  };
  
  double sampleRate = 44100.0;
  bool constQ = false;
  
  double fc = 100.0;
  double Q = DEFAULT_Q;
  double db = 0.0;
};

#endif //AUDIO_FILTER_H
