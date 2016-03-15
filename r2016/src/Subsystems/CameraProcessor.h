/*
 * CameraProcessor.h
 *
 *  Created on: Jan 16, 2016
 *      Author: FIRSTMentor
 */

#ifndef SRC_SUBSYSTEMS_CAMERAPROCESSOR_H_
#define SRC_SUBSYSTEMS_CAMERAPROCESSOR_H_
#include "WPILib.h"
#include "NetworkTables/NetworkTable.h"
#include <SubsystemBase.h>

class CameraProcessor: public SubsystemBase {
private:
	const int k_resX = 320;
	const int k_resY = 240;
	const int k_FOV = 37.4;
	const int k_tWidthIn = 20;
	const int k_tHeightIn = 12;
	const double k_xOffset = -11.0;
	const double k_yOffset = -8.5;
	double m_OffsetAngle = .5;
	bool m_targetVisible;
	bool m_onTarget;
	double m_angle;
	int m_prevOwlCounter;
	int m_owlMissingCounter;
	int m_robotCounter;
	double m_shotRange = 190;
	std::shared_ptr<NetworkTable> m_table;
	Solenoid* m_cameraLight;
public:

	CameraProcessor();
	virtual ~CameraProcessor();
	bool isTargetAvailable();
	bool isOnTarget();
	double getAngle();
	void incOffsetAngle();
	void decOffSetAngle();
	void calculate();
	void SetLight(bool state);
	void Periodic();
};

#endif /* SRC_SUBSYSTEMS_CAMERAPROCESSOR_H_ */
