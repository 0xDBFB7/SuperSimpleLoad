#include <SmoothThermistor.h>

#include <PID_v1.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_MCP4725.h>

Adafruit_MCP4725 dac;
SmoothThermistor smoothThermistor(A2,              // the analog pin to read from
                                  ADC_SIZE_10_BIT, // the ADC size
                                  10000,           // the nominal resistance
                                  10000,           // the series resistance
                                  3950,            // the beta coefficient of the thermistor
                                  25,              // the temperature for nominal resistance
                                  10);             // the number of samples to take for each measurement

uint32_t load_output = 0;
int fan = 0;
float temp = 0;
float voltage = 0;

float pid_current = 0;
float pid_setpoint = 0;

float set_current = 0;
float actual_current = 0;

int initial_current = 0;

String input = "";

PID loadPID(&pid_current, &load_output, &pid_setpoint,2,5,1, DIRECT);

void setup() {
  Serial.begin(115200);
  pinMode(A3,OUTPUT);
  dac.begin(0x62);
  dac.setVoltage(0,false);
  initial_current = analogRead(A0);
  loadPID.SetOutputLimits(0,4095);
  loadPID.SetMode(AUTOMATIC);
}

void loop() {
  if(Serial.available()){
    input = Serial.readStringUntil('\n');
    if(input[0] == 'I'){
      set_current = input.substring(1).toFloat();
    }
    if(input[0] == 'O'){
      dac.setVoltage(input.substring(1).toInt(),false);
    }
  }
  temp = smoothThermistor.temperature();
  actual_current = (analogRead(A0)-initial_current)/9.207;
  voltage = analogRead(A1)/18.61;
  loadPID.Compute();
  dac.setVoltage(load_output,false);
  if(voltage*actual_current > 15 || temp > 50){
    digitalWrite(A3,HIGH);
  }
  else{
    digitalWrite(A3,LOW);
  }
  Serial.println("{\"current\":" + String(actual_current) + ",\"Temperature\":" + String(temp) + ",\"voltage\":" + String(voltage) + ",\"power\":" voltage*actual_current + "}");
}
