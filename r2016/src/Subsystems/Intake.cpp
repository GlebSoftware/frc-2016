/*
 * Intake.cpp
 *
 *  Created on: Jan 18, 2016
 *      Author: FIRSTMentor
 */

#include <Subsystems/Intake.h>
#include "RobotMap.h"
#include <SubsystemBase.h>

Intake::Intake() : SubsystemBase("Intake") {
	m_intakeMotor = new CANTalon(INTAKE_MOTOR);
	m_extender = new Solenoid(INTAKE_EXTENDER);
}

Intake::~Intake() {
	// TODO Auto-generated destructor stub
}

bool Intake::IsLowered() {
	return m_extender->Get();
}

void Intake::Periodic(){
	SmartDashboard::PutNumber("Intake Motor", m_intakeMotor->Get());
	SmartDashboard::PutNumber("Intake Output Current", m_intakeMotor->GetOutputCurrent());
	SmartDashboard::PutNumber("Intake Extender", m_extender->Get());
}
void Intake::Lower() {
	m_extender->Set(true);
}

void Intake::Raise() {
	m_extender->Set(false);
}

void Intake::SetSpeed(double spd) {
	m_intakeMotor->Set(spd);
}

void Intake::TurnOff() {
	m_intakeMotor->Set(0);
}
