#include "WPILib.h"
//#include <Compressor.h>
#define LOOP_SECONDS 50
class Robot: public IterativeRobot
{
	SendableChooser *chooser;
	RobotDrive myRobot; // robot drive system
	Joystick stick; // only joystick
	JoystickButton *btnA;
	JoystickButton *btnB;
	JoystickButton *btnX;
	JoystickButton *btnY;
	JoystickButton *btnLB;
	JoystickButton *btnRB;
	DigitalInput   *swA;
	DigitalInput   *swB;
	DoubleSolenoid *s;
	Compressor *c;
	VictorSP *ballMotor;
	bool bIsTankDrive;

	LiveWindow *lw;
	int autoLoopCounter;
	int autoArmMode;
	int autoBoostMode;

public:
	Robot() :
		chooser(),
		myRobot(0, 1),	// these must be initialized in the same order
		stick(5),		// as they are declared above.
		lw(LiveWindow::GetInstance()),
		autoLoopCounter(0)
	{
		SmartDashboard::init();
		SmartDashboard::PutString("State","Init");

		bIsTankDrive = false;
		SmartDashboard::PutString("Control", "Arcade");

		myRobot.SetExpiration(0.1);

		ballMotor = new VictorSP(2);
		c = new Compressor();
		s = new DoubleSolenoid(0, 1);

		btnA = new JoystickButton(&stick,1);
		btnB = new JoystickButton(&stick,2);
		btnX = new JoystickButton(&stick,3);
		btnY = new JoystickButton(&stick,4);
		btnLB = new JoystickButton(&stick,5);
		btnRB = new JoystickButton(&stick,6);

		swA = new DigitalInput(8);
		swB = new DigitalInput(9);
	}

private:
	void AutonomousInit()
	{
		SmartDashboard::PutString("State","Auto");
		autoLoopCounter = 0;

		s->Set(DoubleSolenoid::Value::kOff);

		autoArmMode = 0;
		SmartDashboard::PutString("AUTOMODE","ARMHIGH");
		if( swA->Get() ){
			autoArmMode = 1;
			SmartDashboard::PutString("AUTOMODE","ARMLOW");
		}

		autoBoostMode = 0;
		SmartDashboard::PutString("BOOSTMODE","BOOST OFF");
		if( swB->Get() ){
			autoBoostMode = 1;
			SmartDashboard::PutString("BOOSTMODE","BOOST ON");
		}
	}

	void AutonomousPeriodic()
	{	//Period ~= 20ms

		if( autoArmMode == 1 && autoBoostMode == 0 ){
			// Low Bar / Port de' Culis
			if(autoLoopCounter < (LOOP_SECONDS * 4)) //Check if we've completed 100 loops (approximately 2 seconds)
			{
				myRobot.Drive(0.0, 0.0); 	//
				s->Set(DoubleSolenoid::Value::kReverse);	//lower arms
				autoLoopCounter++;
			} else if( autoLoopCounter < (LOOP_SECONDS * 7)) {
				myRobot.Drive(0.4, 0.0); 	// drive forwards half speed
				autoLoopCounter++;
			} else if( autoLoopCounter < (LOOP_SECONDS * 10)) {
				s->Set(DoubleSolenoid::Value::kForward);	//raise arms
				myRobot.Drive(0.4, 0.0); 	// drive forwards half speed
				autoLoopCounter++;
			} else {
				myRobot.StopMotor();
				myRobot.Drive(0.0, 0.0); 	// stop robot
			}
		}else if( autoBoostMode == 1 && autoArmMode == 0){
			// Rock Wall / Moat
			if(autoLoopCounter < (LOOP_SECONDS * 2.5)) //Check if we've completed 100 loops (approximately 2 seconds)
			{
				myRobot.Drive(0.6, 0.0); 	// drive forwards half speed
				autoLoopCounter++;
			} else {
				myRobot.StopMotor();
				myRobot.Drive(0.0, 0.0); 	// stop robot
			}
		}else if( autoBoostMode == 1 && autoArmMode == 1 ){
			// Drawbridge / Sally Port
			if(autoLoopCounter < (LOOP_SECONDS * 3)) //Check if we've completed 100 loops (approximately 2 seconds)
			{
				myRobot.Drive(0.15, 0.0); 	// drive forwards half speed
				autoLoopCounter++;
			} else {
				myRobot.StopMotor();
				myRobot.Drive(0.0, 0.0); 	// stop robot
			}
		}else{
			//Rampart / Rough Terrain
			if(autoLoopCounter < (LOOP_SECONDS * 4)) //Check if we've completed 100 loops (approximately 2 seconds)
			{
				myRobot.Drive(0.4, 0.0); 	// drive forwards half speed
				autoLoopCounter++;
			} else {
				myRobot.StopMotor();
				myRobot.Drive(0.0, 0.0); 	// stop robot
			}
		}
	}

	void TeleopInit()
	{
		SmartDashboard::PutString("State","Tele");
		s->Set(DoubleSolenoid::Value::kOff);
		ballMotor->Set(0.0);
	}

	void TeleopPeriodic()
	{
		//X Sets Tank Mode
		//Y Sets Arcade Mode
		if( btnX->Get() ){
			bIsTankDrive = true;
			SmartDashboard::PutString("Control", "Tank");
		}else if( btnY->Get() ){
			bIsTankDrive = false;
			SmartDashboard::PutString("Control", "Arcade");
		}

		if( bIsTankDrive ){
			//Left Bumper = Turbo
			if( btnLB->Get() ){
				myRobot.TankDrive(-stick.GetY(), -stick.GetRawAxis(5), true );
			}else{
				myRobot.TankDrive(-stick.GetY()*0.65, -stick.GetRawAxis(5)*0.65, true );
			}
		}else{
			//Left Bumper = Turbo
			if( btnLB->Get() ){
				myRobot.ArcadeDrive( (float) -stick.GetY(), (float) -stick.GetX() );
			}else{
				myRobot.ArcadeDrive( (float) -stick.GetY()*0.7, (float) -stick.GetX()*0.7 );
			}
		}

		//Right Trigger = Deploy Boulder
		//Right Bumper = Capture Boulder
		if( stick.GetRawAxis(3) > 0.5 ){
			//eject ball
			ballMotor->Set(1.0);
		}else if( btnRB->Get() ){
			//capture ball
			ballMotor->Set(-1.0);
		}else{
			ballMotor->Set(0.0);
		}

		//Left Trigger = Arms Down   (else Arms Up)
		if( stick.GetRawAxis(2) > 0.5 ){
			s->Set(DoubleSolenoid::Value::kReverse);
		}else{
			s->Set(DoubleSolenoid::Value::kForward);
		}

		c->SetClosedLoopControl(true);
	}


	void TestPeriodic()
	{
		SmartDashboard::PutString("State","Test");
		lw->Run();
	}

	void DisabledInit()
	{
		SmartDashboard::PutString("State","Disabled");
	}

	void DisabledPeriodic()
	{

	}

	void Disabled()
	{
		while(IsDisabled()){}
	}

	void RobotInit()
	{
		chooser = new SendableChooser();

		//the camera name (ex "cam0") can be found through the roborio web interface
		// http://roboRIO-6026-FRC.local
		CameraServer::GetInstance()->SetQuality(50);
		CameraServer::GetInstance()->StartAutomaticCapture("cam1");
	}

};

START_ROBOT_CLASS(Robot)
