#include "FusionEKF.h"
#include <iostream>
#include "Eigen/Dense"
#include "tools.h"

using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::cout;
using std::endl;
using std::vector;

/**
 * Constructor.
 */
FusionEKF::FusionEKF() {
  is_initialized_ = false;

  previous_timestamp_ = 0;

  // initializing matrices
  R_laser_ = MatrixXd(2, 2);
  R_radar_ = MatrixXd(3, 3);
  H_laser_ = MatrixXd(2, 4);
  Hj_ = MatrixXd(3, 4);

  //measurement covariance matrix - laser
  R_laser_ << 0.0225, 0,
              0, 0.0225;

  //measurement covariance matrix - radar
  R_radar_ << 0.09, 0, 0,
              0, 0.0009, 0,
              0, 0, 0.09;
// from section 10 lesson 5
  H_laser_ <<1 ,0,0,0,
  			0,1,0,0;
  // the initial transition matrix F_
 ekf_.F_ = MatrixXd(4, 4);
 ekf_.F_ << 1, 0, 1, 0,
      	   0, 1, 0, 1,
           0, 0, 1, 0,
           0, 0, 0, 1;

  // state covariance matrix P	
 ekf_.P_ = MatrixXd(4, 4);
 ekf_.P_ << 1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1000, 0,
            0, 0, 0, 1000;
  
  // set the acceleration noise components
  noise_ax = 5;
  noise_ay = 5;
  /**
   * TODO: Finish initializing the FusionEKF.
   * TODO: Set the process and measurement noises
   */


}

/**
 * Destructor.
 */
FusionEKF::~FusionEKF() {}

void FusionEKF::ProcessMeasurement(const MeasurementPackage &measurement_pack) {
  /**
   * Initialization
   */
  
  if (!is_initialized_) {
    /**
     * TODO: Initialize the state ekf_.x_ with the first measurement.
     * TODO: Create the covariance matrix.
     * You'll need to convert radar from polar to cartesian coordinates.
     */

    // first measurement
    cout << "EKF: " << endl;
    ekf_.x_ = VectorXd(4);
    ekf_.x_ << 0.6, 0.6, 5.2, 0.0; //this value is important for rmse.. can be modified

    if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
      // TODO: Convert radar from polar to cartesian coordinates 
      //         and initialize state.
  	  // Coordinates convertion from polar to cartesian   
		// Initialize from the Radar data set      

      float l = measurement_pack.raw_measurements_[0]; //range (rho)
      float angle = measurement_pack.raw_measurements_[1]; // bearing(phi)
      float l_velocity = measurement_pack.raw_measurements_[2]; //radial velocity(rho dot)
      ekf_.x_ << cos(angle) * l,
                 sin(angle) * l,
                 cos(angle) * l_velocity,
                 sin(angle) * l_velocity;
         
    }
    else if (measurement_pack.sensor_type_ == MeasurementPackage::LASER) {
      // TODO: Initialize state.
      // Initialize from the Lidar data set
      cout << "EKF : First measurement LASER" << endl;      
            ekf_.x_ << measurement_pack.raw_measurements_[0],
                 measurement_pack.raw_measurements_[1],
                 0,
                 0;
    }
    
    // Saving first timestamp in seconds
    previous_timestamp_ = measurement_pack.timestamp_ ;
    // done initializing, no need to predict or update
    is_initialized_ = true;
    return;
  }

  /**
   * Prediction
   */
  // compute the time elapsed between the current and previous measurements
  // dt - expressed in seconds
  float dt = (measurement_pack.timestamp_ - previous_timestamp_) / 1000000.0;
  previous_timestamp_ = measurement_pack.timestamp_;
  
  // 1. F matrix so that the time is integrated
  ekf_.F_ << 1, 0, dt, 0,
          0, 1, 0, dt,
          0, 0, 1, 0,
          0, 0, 0, 1;
  
  // 2. Set the process covariance matrix Q
    double dt_4 = (dt * dt * dt * dt)/4;
    double dt_3 = (dt * dt * dt)/2;

  
    ekf_.Q_ = MatrixXd(4, 4);
    ekf_.Q_ << dt_4*noise_ax,0,dt_3*noise_ax,0,
              0,dt_4*noise_ay,0,dt_3*noise_ay,
              dt_3*noise_ax,0, dt*dt*noise_ax,0,
              0,dt_3*noise_ay,0,dt*dt*noise_ay;
  
  /**
   * TODO: Update the state transition matrix F according to the new elapsed time.
   * Time is measured in seconds.
   * TODO: Update the process noise covariance matrix.
   * Use noise_ax = 9 and noise_ay = 9 for your Q matrix.
   */

  ekf_.Predict();

  /**
   * Update
   */

  /**
   * TODO:
   * - Use the sensor type to perform the update step.
   * - Update the state and covariance matrices.
   */
  // Test for Only Lidar or Only Radar or both
  FusionEKF fusionEKF;
  fusionEKF.Choosesensor_type_ = BOTH_;
  
  
  if ((measurement_pack.sensor_type_ == MeasurementPackage::RADAR) && (fusionEKF.Choosesensor_type_ == BOTH_)) {
    // TODO: Radar updates
    ekf_.H_ = tools.CalculateJacobian(ekf_.x_);
  	ekf_.R_ = R_radar_;
  	ekf_.UpdateEKF(measurement_pack.raw_measurements_);

  } else if ((measurement_pack.sensor_type_ == MeasurementPackage::LASER) && (fusionEKF.Choosesensor_type_ == BOTH_)) {
    // TODO: Laser updates
    ekf_.H_ = H_laser_;
  	ekf_.R_ = R_laser_;
  	ekf_.Update(measurement_pack.raw_measurements_);
  } else if ((measurement_pack.sensor_type_ == MeasurementPackage::RADAR) && (fusionEKF.Choosesensor_type_ == RADAR_)) {
    // TODO: Radar updates
    ekf_.H_ = tools.CalculateJacobian(ekf_.x_);
  	ekf_.R_ = R_radar_;
  	ekf_.UpdateEKF(measurement_pack.raw_measurements_);
  } else if ((measurement_pack.sensor_type_ == MeasurementPackage::LASER) && (fusionEKF.Choosesensor_type_ == LASER_)) {
        // TODO: Laser updates
    ekf_.H_ = H_laser_;
  	ekf_.R_ = R_laser_;
  	ekf_.Update(measurement_pack.raw_measurements_);
  } else {
    //do nothing
  }
    
  // print the output
  cout << "x_ = " << ekf_.x_ << endl;
  cout << "P_ = " << ekf_.P_ << endl;
}