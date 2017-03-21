#include <OneWire.h> 
#include <DallasTemperature.h>
OneWire oneWire(2); 
DallasTemperature sensors(&oneWire);
// constants
const float SENSOR_CALIBRATION_VALUE = 0.004882814;
const int DESIRED_TEMP = 17;
const int TOP_FAN_SPEED = 255;

const byte GREEN_LED = 9;
const byte RED_LED = 8;
const byte BLUE_LED = 7;
const byte PELTIER = 4;
const byte FANS = 3;
//const byte TEMP_SENSOR = A0;

const int LED_BLINK_DELAY = 1000;
const int TEMP_CHECK_DELAY = 2000;

int state;
int fanPWM = 0;

enum states
{
	idle = 0,
	cooling,
	error
};

void setup(void)
{
  Serial.begin(9600);
  state = idle;
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(PELTIER, OUTPUT);
  pinMode(FANS, OUTPUT);
  sensors.begin();
}

void loop(void)
{
	switch(state)
	{
		case idle:
		{
			doIdle();
			state = cooling;
			break;
		}
		case cooling:
		{
			int result = -1;

			if(!doCooling())
			{
				state = error;
			}
			else
			{
				state = idle;
			}
			break;
		}
		case error:
		{
			doError();
			break;
		}
		default:
		{
			break;
		}
	}
}

void doIdle(void)
{
  Serial.println("Idle");
  _T("Fan", fanPWM);
	digitalWrite(GREEN_LED, HIGH);
	float temp = readTemp();
	while( DESIRED_TEMP > temp ) //check we are in desired temp
	{
		if(DESIRED_TEMP < temp - 1)
		{
			// only to inform we are below desired temp
			digitalWrite(BLUE_LED, HIGH);
		}
		else
		{
			digitalWrite(BLUE_LED, LOW);
		}

		slowCoolerIfMooving();

		delay(TEMP_CHECK_DELAY);
    temp = readTemp();
	}

	digitalWrite(BLUE_LED, LOW);// not cooler than expected for sure
	digitalWrite(GREEN_LED, LOW);
}

int doCooling(void)
{
	Serial.println("Cooling");
	digitalWrite(RED_LED, HIGH);
	analogWrite(FANS, TOP_FAN_SPEED);//turn on fans at full speed
	digitalWrite(PELTIER, HIGH);// turn on cooling

	float temp = readTemp();
	while( DESIRED_TEMP < temp ) //check we are hot
	{
		// stay here for as long as it takes to reach the desired temp

		// TODO: implement time count to deside it's been too long cooling
		/* TODO implement RPM lectures
		if(fanRPM == 0)
		{
			// fan is still, something is wrong!
			digitalWrite(PELTIER, LOW);// turn on cooling
			digitalWrite(FANS, LOW);// turn off fans
			state = error;
			break;
		}
		*/

		delay(TEMP_CHECK_DELAY);
		temp = readTemp();
	}

	digitalWrite(RED_LED, LOW); // no more on cooling state
	return 0;
}

void doError(void)
{
	digitalWrite(PELTIER, LOW); //in error, first thing is to protect the peltier
	Serial.println("Error");
	bool ledState = 0;
	while(1)
	{
		ledState = !ledState;
		digitalWrite(RED_LED, ledState);
		slowCoolerIfMooving();

		delay(LED_BLINK_DELAY);
		// TODO: get out if been here for too long
	}
}

float readTemp(void)
{
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
  /*
  float sensor = analogRead(TEMP_SENSOR);
  _T("sensor", sensor);
  float voltageFromSensor = sensor * 0.004882814;
  _T("voltageFromSensor", voltageFromSensor);
  float celsiusTemp = (voltageFromSensor - 0.5) * 100.0;
  _T("celsiusTemp", celsiusTemp);
  return celsiusTemp;
  */
}

void _T(String text, float value)
{
  Serial.print(text + " :");
  Serial.println(value);
}

void slowCoolerIfMooving(void)
{
	if(fanPWM > 0)
	{
		// if fan is on but temp is desired, decreese it's speed slowly
		fanPWM =- 5;
		analogWrite(FANS, fanPWM);
	}
}