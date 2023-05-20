#include "AudioFilter.h"


double PeakingFilter::processAudioSample(double xn, uint8_t ch) {
  double x_z1 = states[ch][0];
  double x_z2 = states[ch][1];
  double y_z1 = states[ch][2];
  double y_z2 = states[ch][3];
  
  double yn = cf_a0 * xn + cf_a1 * x_z1 + cf_a2 * x_z2 - cf_b1 * y_z1 - cf_b2 * y_z2;

  checkFloatUnderflow(yn);

  x_z2 = x_z1;
  x_z1 = xn;

  y_z2 = y_z1;
  y_z1 = yn;
  
  states[ch][0] = x_z1;
  states[ch][1] = x_z2;
  states[ch][2] = y_z1;
  states[ch][3] = y_z2;

  return cf_d0 * xn + cf_c0 * yn;
}


void PeakingFilter::calculateFilterCoeffs() {
  resetCoeffs();

  // Non constant Q
  if (!constQ) {
    double theta_c = 2.0 * kPi * fc / sampleRate;
    double mu = pow(10.0, db / 20.0);

    double tanArg = theta_c / (2.0 * Q);
    if (tanArg >= 0.95 * kPi / 2.0) tanArg = 0.95 * kPi / 2.0;

    double zeta = 4.0 / (1.0 + mu);
    double betaNumerator = 1.0 - zeta * tan(tanArg);
    double betaDenominator = 1.0 + zeta * tan(tanArg);

    double beta = 0.5 * (betaNumerator / betaDenominator);
    double gamma = (0.5 + beta) * (cos(theta_c));
    double alpha = (0.5 - beta);

    cf_a0 = alpha;
    cf_a1 = 0.0;
    cf_a2 = -alpha;
    cf_b1 = -2.0 * gamma;
    cf_b2 = 2.0 * beta;

    cf_c0 = mu - 1.0;
    cf_d0 = 1.0;
  }
  else {
    // Constant Q
    double K = tan(kPi * fc / sampleRate);
    double Vo = pow(10.0, db / 20.0);

    double d0 = 1.0 + (1.0 / Q) * K + K * K;
    double e0 = 1.0 + (1.0 / (Vo * Q)) * K + K * K;
    double alpha = 1.0 + (Vo / Q) * K + K * K;
    double beta = 2.0 * (K * K - 1.0);
    double gamma = 1.0 - (Vo / Q) * K + K * K;
    double delta = 1.0 - (1.0 / Q) * K + K * K;
    double eta = 1.0 - (1.0 / (Vo * Q)) * K + K * K;
    
    if (db >= 0) {
      cf_a0 = alpha / d0;
      cf_a1 = beta  / d0;
      cf_a2 = gamma / d0;
      cf_b1 = beta  / d0;
      cf_b2 = delta / d0;
    }
    else {
      cf_a0 = d0    / e0;
      cf_a1 = beta  / e0;
      cf_a2 = delta / e0;
      cf_b1 = beta  / e0;
      cf_b2 = eta   / e0;
    }
  }
}
