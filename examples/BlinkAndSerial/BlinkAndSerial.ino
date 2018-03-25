/*
 * This example code will blink the built in LED (on pin 13) every 500ms, and 
 * send three serial messages, at 9600 baud, at a rate of 1, 5, and 15 seconds. 
 *
 * This example code is in the public domain.
 */
#include "SimpleTaskScheduler.h"

/*==============================================================================
 * GLOBAL VARIABLES
 *============================================================================*/

#define LED_PIN 13

SimpleTaskScheduler taskScheduler(4, TS_MILLIS);


/*==============================================================================
 * FUNCTIONS
 *============================================================================*/

void toggle_led(void) {
	digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}
 
 void send_1s_packet(void) {
	Serial.println("1 second packet");
}


void send_5s_packet(void) {
	Serial.println("5 second packet");
}


void send_15s_packet(void) {
	Serial.println("15 second packet");
}



/*==============================================================================
 * SETUP()
 *============================================================================*/
void setup()
{
	pinMode(LED_PIN, OUTPUT);
 
	Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
	taskScheduler.addTask(toggle_led, 500, true);
	taskScheduler.addTask(send_1s_packet, 1000, true);
	taskScheduler.addTask(send_5s_packet, 5000, true);
	taskScheduler.addTask(send_15s_packet, 15000, true);
}

/*==============================================================================
 * LOOP()
 *============================================================================*/
void loop()
{
  taskScheduler.loop();
}
