#include <SoftwareSerial.h>
#include "LowPower.h"
#include "millisDelay.h"
#include "version.h"
#include "Xbee_lib.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Streaming.h>
#include <Vector.h>

Vector<uint8_t> vector;
uint8_t vector_storage[30];

#define LED_PIN     13
#define WAKE_PIN     3
#define DS180_TEMP   4
#define VACUUM      A6   // 20
#define BATTERY     A7   // 21
#define OUT_PIN      6

SoftwareSerial ss(7,8);  // (rx,tx)
Xbee_lib m_xbee(&ss);

struct Msg_data m_rx_msg;

millisDelay m_send_timer;
millisDelay m_sleep_timer;
millisDelay m_wireless_timer;

OneWire oneWire(DS180_TEMP);
DallasTemperature sensor(&oneWire);

bool m_sleep_now = true;
bool m_tx_now = true;
uint8_t m_tx_count = 0;

//////////////////////////////////////////////////////////////////////
 
void setup() 
{ 
  // allow time to switch to xbee mode on pcb
  delay(2000);

  Serial.begin(19200);
  Serial.println("**** SERIAL ****");

  ss.begin(19200);   //m_xbee.Begin(19200);
  ss.print("xbee_api_sleep_txrx_usb_remote : ");
  ss.println(version);


  // pin definitions
  pinMode(WAKE_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(OUT_PIN, OUTPUT); digitalWrite(OUT_PIN, LOW);

  // delay before transmission is sent again (no response)
  m_send_timer.start(2000);

  // sleep after 10 seconds regardless if transmission/response status
  m_sleep_timer.start(10000);

  // slow the tx-ing and rx-ing handling loop
  m_wireless_timer.start(1);

  // Start up the library for dallas temp
  sensor.begin();

  // callback for when valid data received
  m_xbee.Set_callback(Message_received);

  vector.setStorage(vector_storage);
}  

//////////////////////////////////////////////////////////////////////

void wakeUp()
{
  // called after interrupt (no delays or millis)
  // reset variables after wakeup
  m_tx_now = true;
  m_sleep_now = false;
}

//////////////////////////////////////////////////////////////////////
  
void loop() 
{ 
  handle_sleep(m_sleep_now);

  // slow the wireless loop
  if(m_wireless_timer.justFinished())
  {
    m_wireless_timer.repeat();
    handle_wireless();
  }
  
  // no successful response, sleep anyway
  if(m_sleep_timer.justFinished())
  {
    ss.println("wireless timeout, going to sleep");
    m_sleep_now = true;
  }
} 

//////////////////////////////////////////////////////////////////////

void handle_sleep(bool sleep)
{
  // put micro to sleep
  if(sleep)
  {
    // safe state?
    digitalWrite(LED_PIN, LOW);
    //digitalWrite(OUT_PIN, LOW);

    // attach external interrupt and then sleep
    attachInterrupt(digitalPinToInterrupt(WAKE_PIN), wakeUp, RISING);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

    //delay after wakeup
    delay(100);
    digitalWrite(LED_PIN, HIGH);

    // Disable external pin interrupt on wake up pin.
    detachInterrupt(digitalPinToInterrupt(WAKE_PIN));

    m_sleep_timer.restart();
  }
}

//////////////////////////////////////////////////////////////////////

void handle_wireless()
{ 
  while(Serial.available())
  {
    m_xbee.Process_byte(Serial.read());
  }

  // build message, insert payloads
  struct Msg_data tx_msg;
  tx_msg.length = 23;
  tx_msg.frame_type = 0x10;
  tx_msg.address = ID::XBEE_1;
  tx_msg.payload_cnt = m_tx_count;
  tx_msg.payload_id = CMD_ID::IO_IN;

  // light sensor
  analogRead(A2); // throw away
  uint16_t light = analogRead(A2);
  tx_msg.payload[0] = light/4;
  tx_msg.payload_v.push_back(light/4);

  // temp sensor
  tx_msg.payload[1] = getDallasTemp();
  tx_msg.payload_v.push_back(getDallasTemp());

  // battery
  analogRead(A7); // throw away
  uint16_t battery = analogRead(A7);
  tx_msg.payload[2] = battery/4;
  tx_msg.payload_v.push_back(battery/4);

  // transmit data, timer has timed out
  if(m_tx_now &&  m_send_timer.justFinished())
  {
    // use enum from transmit status
    uint8_t tx_ok = m_xbee.Transmit_data(tx_msg);
    if(tx_ok == 1)
    {
      m_tx_count++;
    }

    // reset timer
    m_send_timer.repeat();
    m_tx_now = false;
  }
}

//////////////////////////////////////////////////////////////////////

uint8_t Message_received(const struct Msg_data rx_data)
{
  ss.println("Message_received");
  m_xbee.Print_msg(rx_data);

  switch(rx_data.payload_id)
  {
    case CMD_ID::ACK :
      ss.println("Rx'd Ack");
      m_sleep_now = true;
      break;

    case CMD_ID::IO_OUT :
      ss.println("Rx'd IO out");
      m_sleep_now = run_outputs(rx_data.payload);   // s3o3611
      break;

    default :
      ss.print("Unknown CMD::ID: ");
      ss.println(rx_data.payload_id);
      m_sleep_now = true;
  }

  // received valid response, do stuff and then sleep
  m_sleep_now = true;
}

bool run_outputs(const uint8_t payload[]) // should be constantly run from main loop
{
  bool sleep = false;
  int pin = payload[0];
  int state = payload[1];
  int time = payload[2];

  digitalWrite(pin, state);
  if(time != 0)
  {
    delay(time * 1000);
    digitalWrite(pin, LOW);
  }
  return sleep;
}

//////////////////////////////////////////////////////////////////////

uint8_t getDallasTemp()
{
  delay(10);
  sensor.requestTemperatures(); // Send the command to get temperatures
  float tempF = (sensor.getTempCByIndex(0) * 9.0 / 5.0) + 32;

  if(tempF < 0)
  {
    tempF = 0;
  }
  return tempF;
}
