#include "Shooter.h"
#include "../RobotMap.h"
#include "../RoboPreferences.h"

Shooter::Shooter() :
		SubsystemBase("Shooter"), m_notifier(&Shooter::PeriodicTask, this)
{
	m_shooterDistance = 0;
	m_shooterSpeed = 3100;
	m_onTargetCounter = 0;
	m_highPosition = true;
	m_shooterWheel = new CANTalon(SHOOTER_MOTOR);
	m_shooterWheel->SetClosedLoopOutputDirection(true);
	m_shooterAdjuster = new Solenoid(SHOOTER_ANGLE_ADJUST);
	SmartDashboard::PutBoolean("Shooter Tuning", false);
	m_shooterWheel->SelectProfileSlot(0);
	m_shooterWheel->SetControlMode(CANTalon::kSpeed);
	m_shooterWheel->SetPID(2.04, 0.0, 0.0, .03589);
	m_shooterWheel->SetFeedbackDevice(CANTalon::CtreMagEncoder_Relative);
	m_shooterWheel->SetSensorDirection(true);
	//TODO: Move to DriveTrain once Working
	m_notifier.StartPeriodic(0.005);
	m_loopTimeoutMP = -1;
}

void Shooter::InitDefaultCommand()
{
}

void Shooter::Periodic() {
	SmartDashboard::PutNumber("Shooter Motor", m_shooterWheel->GetSpeed());
	SmartDashboard::PutNumber("Shooter Enc Vel", m_shooterWheel->GetEncVel());
	SmartDashboard::PutNumber("Shooter Error", m_shooterWheel->GetClosedLoopError());
	SmartDashboard::PutNumber("Shooter Setpoint", m_shooterSpeed);
	SmartDashboard::PutBoolean("Shooter High Position", m_highPosition);
	SmartDashboard::PutBoolean("Shooter on Target", IsOnTarget());
	SmartDashboard::PutNumber("Shooter Current", m_shooterWheel->GetOutputCurrent());
	PeriodicMotionProfile();
}

void Shooter::TurnOff(){
	m_shooterWheel->Disable();
}

void Shooter::SetShooterSpeed(bool forward) {
	m_shooterWheel->SetControlMode(CANTalon::kSpeed);
	if (forward){
		m_shooterWheel->Enable();
		m_shooterWheel->Set(m_shooterSpeed);
	}
	else {
		m_shooterWheel->Enable();
		m_shooterWheel->Set(-m_shooterSpeed);
	}
}

void Shooter::SetGoalDistance(double val) {
	m_shooterDistance = val;
}

double Shooter::GetShooterSpeed() {
	return m_shooterSpeed;
}

double Shooter::GetGoalDistance() {
	return m_shooterDistance;
}

bool Shooter::IsOn() {
	return m_shooterWheel->IsEnabled();
}

double Shooter::GetDesiredSpeed() {
	return m_shooterWheel->GetSetpoint();
}

void Shooter::SetHighPosition() {
	m_shooterAdjuster->Set(false);
	m_highPosition = true;
}

void Shooter::SetLowPosition() {
	m_shooterAdjuster->Set(true);
	m_highPosition = false;
}

bool Shooter::IsOnTarget() {
	if (m_shooterWheel->GetSpeed() > 1000 && fabs(m_shooterWheel->GetSpeed() - m_shooterWheel->GetSetpoint()) < 20) {
		m_onTargetCounter++;
	}
	else {
		m_onTargetCounter = 0;
	}

	return m_onTargetCounter > 10;
}

bool Shooter::GetPosition() {
	return m_highPosition;
}

void Shooter::incShooterSpeed() {
	m_shooterSpeed += 100;
}

void Shooter::decShooterSpeed() {
	m_shooterSpeed -= 100;
}

//TODO: Move to DriveTrain once working
void Shooter::PeriodicTask(){
	m_shooterWheel->ProcessMotionProfileBuffer();
}

void Shooter::ResetMotionControl() {
	m_shooterWheel->ClearMotionProfileTrajectories();
}

void Shooter::StartFilling(){
	StartFilling(kMotionProfile,kMotionProfilesz);
}

void Shooter::StartMotionProfile() {
	m_shooterWheel->Enable();
	m_shooterWheel->SetControlMode(CANTalon::kMotionProfile);
	m_startMP = true;
}

void Shooter::StopMotionProfile() {
	m_shooterWheel->Set(CANTalon::SetValueMotionProfileDisable);
//	m_shooterWheel->SetControlMode(CANTalon::kSpeed);
}

void Shooter::PeriodicMotionProfile(){
	m_shooterWheel->GetMotionProfileStatus(m_motionProfileStatus);

	if(m_loopTimeoutMP >= 0){
		if(m_loopTimeoutMP == 0){
			instrumentation::OnNoProgress();
			//TODO: we might want to do somthing here so we can react to the talon not being there
		} else {
			m_loopTimeoutMP--;
		}
	}

	if(m_shooterWheel->GetControlMode() != CANSpeedController::kMotionProfile){
		m_stateMP = 0;
	} else {
		switch (m_stateMP){
	case 0:
			if (m_startMP){
				m_startMP = false;

//				m_setValueMP = CANTalon::SetValueMotionProfileDisable;
				m_shooterWheel->Set(CANTalon::SetValueMotionProfileDisable);
				StartFilling();

				m_stateMP = 1;
				m_loopTimeoutMP = kNumLoopsTimeoutMP;
			}
			break;
		case 1:
			if(m_motionProfileStatus.btmBufferCnt > kMinPointsInTalonMP){
//				m_setValueMP = CANTalon::SetValueMotionProfileEnable;
				m_shooterWheel->Set(CANTalon::SetValueMotionProfileEnable);
				m_stateMP = 2;
				m_loopTimeoutMP = kNumLoopsTimeoutMP;
			}
			break;
		case 2:
			if(m_motionProfileStatus.isUnderrun == false){
				m_loopTimeoutMP = kNumLoopsTimeoutMP;
			}

			if(m_motionProfileStatus.activePointValid && m_motionProfileStatus.activePoint.isLastPoint){
//				m_setValueMP = CANTalon::SetValueMotionProfileHold;
				m_shooterWheel->Set(CANTalon::SetValueMotionProfileHold);
				m_stateMP = 0;
				m_loopTimeoutMP = -1;
			}
			break;
		}
	}
//	m_shooterWheel->Set(m_setValueMP);
	instrumentation::Process(m_motionProfileStatus);
}

bool Shooter::IsMPFinished(){
	return m_motionProfileStatus.activePointValid && m_motionProfileStatus.activePoint.isLastPoint;
}

void Shooter::StartFilling(const double profile[][3],int totalCnt){
	CANTalon::TrajectoryPoint point;
	//TODO: Handle Underrun

	m_shooterWheel->ClearMotionProfileTrajectories();

	for (int i=0; i < totalCnt; i++){
		point.position = profile[i][0];
		point.velocity = profile[i][1];
		point.timeDurMs = profile[i][2];

		point.profileSlotSelect = 0;
		point.velocityOnly = false;

		point.zeroPos = false;
		if(i == 0){
			point.zeroPos = true;
		}

		point.isLastPoint = false;
		if (i + 1 == totalCnt){
			point.isLastPoint = true;
		}

		m_shooterWheel->PushMotionProfileTrajectory(point);
	}
}

CANTalon::SetValueMotionProfile Shooter::getSetValue(){
	return CANTalon::SetValueMotionProfileDisable; //m_setValueMP;
}
