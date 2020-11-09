#include <ESP8266HTTPClient.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>

const char* ssid = "RouterK";
const char* password = "mostowa18router";

Servo mg995;
OneWire oneWire(2);
DallasTemperature sensors(&oneWire);

float readTemp();
void startSetup();
void receiveConfig();
void sendData();
void sendConfirmation();
float PID();
void setServo();

float temperature_now = 0;
float temperature_set = 23.0;
int PID_status = -1;
int PID_set = 4;
float error_prev;
float error;
int manualMode = 0;
float PID_min = 4.0;
float PID_max = 11.0;

unsigned int sendTempPeriod = 60000;
unsigned int checkPeriod = 10000;
unsigned long checkTime_start;
unsigned long sendTime_start;
unsigned long PIDTime_start;
float PIDTime;

int kp = 5;
int ki = 3500;
int kd = 0;
float I = 0;
float D = 0;

void setup() {
	startSetup();				//podstawowe uruchomienie systemu
	receiveConfig();			// odczytaj konfiguracjê
	temperature_now = readTemp();		//odczyt temperatury
	sendData();
	error_prev = temperature_set - temperature_now;
	I = ki * (((float)PID_set / kp) - error_prev - (kd * D));
	checkTime_start, sendTime_start, PIDTime_start = millis();
}

void loop() {
	if ((millis() - checkTime_start) > checkPeriod) {		// odczyt ustawieñ z serwera co 10 sekund
		Serial.println("Poczatek petli 10-sekundowej");
		receiveConfig();
		if (manualMode) setServo();

		checkTime_start = millis();
	}

	if ((millis() - sendTime_start) > sendTempPeriod) {		// pomiar temperatury i wysylka danych
		Serial.println("Poczatek petli 60-sekundowej");
		temperature_now = readTemp();

		if (!manualMode) {
			Serial.println("Wywo³anie PID-a");
			PIDTime = (millis() - PIDTime_start) / 1000.0;			// w sekundach
			PIDTime_start = millis();
			PID_set = (int) PID();
		}

		setServo();
		sendData();

		sendTime_start = millis();

		if (millis() > 86400000)
			ESP.restart();
	}
}

float readTemp() {									// pomiar temperatury
	sensors.requestTemperatures();
	delay(800);
	return round(sensors.getTempCByIndex(0)*10)/10;
}

void startSetup() {									// zaladowanie przystawek na starcie
	Serial.begin(115200);
	sensors.begin();
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(100);
	}
	Serial.println(WiFi.localIP());
	mg995.attach(14);
	setServo();
}

void receiveConfig() {								// odebranie ustawien z serwera
	boolean configChanged = false;
	HTTPClient client;
	client.setTimeout(5000);
	float temp = readTemp();
	if (client.begin("http://192.168.0.10/sendConfig.php")) {
		client.addHeader("Content-Type", "application/x-www-form-urlencoded");
		//Serial.println("Polaczono z serwerem. Odbior danych...");
		int httpCode = client.GET();
		//Serial.println("HTTP CODE: " + (String) httpCode);
		if (httpCode == 200) {
			const size_t capacity = JSON_OBJECT_SIZE(6) + 80;
			DynamicJsonDocument doc(capacity);
			deserializeJson(doc, client.getString());

			int readKp = doc["kp"];
			int readKi = doc["ki"];
			int readKd = doc["kd"];
			float readTemp = doc["temperature_set"];
			float readPIDSet = doc["pid_set"];
			int readManualMode = doc["manual_mode"];

			if (readKp != kp) {
				kp = readKp;
				Serial.println("Odczytano i zmieniono wartosc kp na: " + (String) kp);
				configChanged = true;
			}

			if (readKi != ki) {
				ki = readKi;
				Serial.println("Odczytano i zmieniono wartosc ki na: " + (String) ki);
				configChanged = true;
			}

			if (readKd != kd) {
				kd = readKd;
				Serial.println("Odczytano i zmieniono wartosc kd na: " + (String) kd);
				configChanged = true;
			}

			if (readTemp != temperature_set) {
				temperature_set = readTemp;
				Serial.println("Odczytano i zmieniono nastawe temperatury na: " + (String) temperature_set);
				configChanged = true;
			}

			if ((int)(2*readPIDSet) != PID_set) {
				Serial.println("Odczytano nastawe zaworu na: " + (String)readPIDSet);
				PID_set = 2*readPIDSet;
				Serial.println("Odczytano i zmieniono nastawe zaworu na: " + (String) PID_set);
				configChanged = true;
			}

			if (readManualMode != manualMode) {
				if (readManualMode)
				{
					error_prev = temperature_set - temp;
					I = ki * (((float) PID_set / kp) - error_prev - (kd * D));
				}
				manualMode = readManualMode;
				Serial.println("Odczytano i zmieniono tryb na: " + (String) manualMode);
				configChanged = true;
			}

			Serial.println("Odebrano dane\n");

			sendConfirmation();

			if (configChanged == true) sendData();
			configChanged = false;

		}
		if (httpCode == 404) {
			Serial.println("UPS! Problem z Internetem");
		}
		client.end();
	}
	else {
		Serial.println("B³¹d po³¹czenia");
	}
	
}

void sendData() {									// wysylka danych na serwer
	HTTPClient client;
	client.setTimeout(5000);
	String request = "temperature_now=" + (String) temperature_now + "&temperature_set=" + temperature_set + "&PID_status=" + (float) PID_status / 2 + "&manual_mode=" + manualMode;
	Serial.println("Przygotowano request: " + request);

	if (client.begin("http://192.168.0.10/readParameters.php")) {
		client.addHeader("Content-Type", "application/x-www-form-urlencoded");
		//Serial.println("Polaczono z serwerem. Wysylka...");
		int httpCode = client.POST(request);
		//Serial.println("HTTP CODE: " + (String)httpCode);
		if (httpCode == 200 ) {
			Serial.println(client.getString() + "\n");
		}

		if (httpCode == 404) {
			Serial.println("UPS! Problem z Internetem");
		}
		client.end();
	}
	else {
		Serial.println("Blad po³¹czenia");
	}
}

void sendConfirmation() {								// wysylka potwierdzenia przyjecia ustawien
	HTTPClient client;
	client.setTimeout(5000);
	if (client.begin("http://192.168.0.10/confirmation.php")) {
		client.addHeader("Content-Type", "application/x-www-form-urlencoded");
		//Serial.println("Polaczono z serwerem. Wysylka potwierdzenia...");
		int httpCode = client.GET();
		//Serial.println("HTTP CODE: " + (String)httpCode);
		if (httpCode == 200) {
			Serial.println(client.getString() + "\n");
		}

		if (httpCode == 404) {
			Serial.println("UPS! Problem z Internetem");
		}
		client.end();
	}
	else {
		Serial.println("Blad po³¹czenia");
	}
}

float PID() {
	error = temperature_set - temperature_now;
	float P = error;
	I += ((error_prev+error)/2.0)*PIDTime;
	D = (error - error_prev) / PIDTime;

	if (I > (PID_max*ki / kp)) I = PID_max * ki / kp;
	if (I < PID_min) I = PID_min;

	float PID = kp*(P + I/ki + kd*D);
	error_prev = error;

	if (PID < PID_min) PID = PID_min;
	if (PID > PID_max) PID = PID_max;

	Serial.println("\nRegulacja P: " + (String)(kp*P));
	Serial.println("Regulacja I: " + (String)(kp*I / ki));
	Serial.println("Regulacja D: " + (String)(kp*kd*D));
	Serial.println("Regulacja PID: " + (String)PID + "\n");

	return PID;
}

void setServo() {
	//Serial.println("Metoda setServo");
	Serial.println("PID_set - " + (String)PID_set);
	Serial.println("PID_status - " + (String)PID_status);
	if (PID_status > PID_set) {
		mg995.writeMicroseconds(map((PID_set - 1), 0, 20, 580, 1400));
		//Serial.println("Wartosc zmiany - " + (String) (PID_set - 1));
		delay(500);
		mg995.writeMicroseconds(map(PID_set, 0, 20, 580, 1400));
		//Serial.println("Wartosc zmiany - " + (String) PID_set);
		PID_status = PID_set;
	}
	else if (PID_status < PID_set) {
		mg995.writeMicroseconds(map((PID_set + 1), 0, 20, 580, 1400));
		//Serial.println("Wartosc zmiany + " + (String)(PID_set + 1));
		delay(500);
		mg995.writeMicroseconds(map(PID_set, 0, 20, 580, 1400));
		//Serial.println("Wartosc zmiany + " + (String)PID_set);
		PID_status = PID_set;
	}
}