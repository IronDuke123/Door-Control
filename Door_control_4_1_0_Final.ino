 /*--------------------------------------------------------------
 Program:      eth_websrv_Door_control
 
 Description:  Arduino web server that shows controls for operating 3 garage doors.
 there is open,stop, and close for each door.
 
 Hardware:     Arduino Uno and official Arduino Ethernet
 shield. Should work with other Arduinos and
 compatible Ethernet shields.
 1Gb micro SD card formatted FAT16.
 D5-D6-D7 for door 1, A0-A2 for door 2, A3-A5 for door 3
 Ethernet shield has reset pin and rest on ICSP header removed.. Reset is wired to D8.. 
 This is to reset ethernet on powerup otherwise you must reset manually everytime..
 
 Software:     Developed using Arduino 1.0.5 software
 Should be compatible with Arduino 1.0 +
 SD card contains web page called index.htm
 
 References:   - WebServer example by David A. Mellis and 
 modified by Tom Dupras  tek1229@yahoo.com
 - SD card examples by David A. Mellis and Tom Igoe
 - Ethernet library documentation:
 http://arduino.cc/en/Reference/Ethernet
 - SD Card library documentation:
 http://arduino.cc/en/Reference/SD
 
 Date:         4 April 2013
 Modified:     11 Jan 2014
 - removed use of the String class
 
 Author:       W.A. Smith, http://startingelectronics.com
 Modified by Thomas Dupras on Jan 22nd,2014
 this revision also checks for ip address and uses the last number to verify authorized users.. not the best but it works for what is needed..
 --------------------------------------------------------------*/

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ   60

// MAC address from Ethernet shield sticker under board
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // should be changed to something else
IPAddress ip(10, 16, 246, 211); // IP address, may need to change depending on network
EthernetServer server(80);  // create a server at port 80
File webFile;               // the web page file on the SD card
char HTTP_req[REQ_BUF_SZ] = {
  0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer
int LED_state[8] = {0}; // stores the states of the LEDs
byte rip[] = { 0,0,0,0 };  // rip[] is the returned ip address of client requesting
boolean key=false;


void setup()
{
  // disable Ethernet chip
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  Serial.begin(9600);       // for debugging
  //Serial.println(F("Ip Address is 10.16.246.11"));
  // initialize SD card
     if (!SD.begin(4)) {
    Serial.println(F("ERROR - SD init failed!"));
    return;    // init failed
  }
  //Serial.println(F("SUCCESS - SD card initialized."));
  // check for index.htm file
  if (!SD.exists("index.htm")) {
    Serial.println(F("ERROR!"));
    return;  // can't find index file
  }
  Serial.println(F("SUCCESS - Found index.htm"));
  // door outputs turning them high before using pinMode keeps them from twitching on startup..
  PORTD = PORTD | B11100000; // sets digital pins 7,6,5 HIGH
  PORTC = PORTC | B00111111; // sets analog pins 2,1,0 HIGH

  
DDRD = DDRD | B11100000; // turns 5,6,7 into outputs..
DDRC = DDRC | B00111111; // turns analog 0-5 into outputs..
  pinMode(8, OUTPUT); // reset ethernet shield output
  digitalWrite(8,LOW); // reset ethernet
  delay(200);
  digitalWrite(8,HIGH); // done resetting
  delay(200);
  pinMode(8, INPUT); // pin 8 into input in case reset is pushed so pin isn't shorted out..
  delay(2000);
  
  
  
  Ethernet.begin(mac, ip);  // initialize Ethernet device
  server.begin();           // start to listen for clients
}

void loop()
{
  EthernetClient client = server.available();  // try to get client

  if (client) {  // got client?
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {   // client data available to read
        char c = client.read(); // read 1 byte (character) from client
        // limit the size of the stored received HTTP request
        // buffer first part of HTTP request in HTTP_req array (string)
        // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
        if (req_index < (REQ_BUF_SZ - 1)) {
          HTTP_req[req_index] = c;          // save HTTP request character
          req_index++;
        }
        // last line of client request is blank and ends with \n
        // respond to client only after last line received
        if (c == '\n' && currentLineIsBlank) {
          client.getRemoteIP(rip); // where rip is defined as byte rip[] = {0,0,0,0 };
          checkIP();

          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          // remainder of header follows below, depending on if
          // web page or XML page is requested
          // Ajax request - send XML file
          if (key== true && StrContains(HTTP_req, "ajax_inputs")) {
            // send rest of HTTP header
            client.println("Content-Type: text/xml");
            client.println("Connection: keep-alive");
            client.println();
            SetLEDs();  // parse the http get request and control outputs accordingly
                     }
          else  {  // web page request
            // send rest of HTTP header
            client.println("Content-Type: text/html");
            client.println("Connection: keep-alive");
            client.println();
            // send web page
            if (key == true) webFile = SD.open("index.htm");        // open web page file
            else webFile = SD.open("BadIP.htm"); // if not autorized by IP then an error page is displayed.. 
            if (webFile) {
              while(webFile.available()) {
                client.write(webFile.read()); // send web page to client
              }
              webFile.close();
            }
          }

          // display received HTTP request on serial port uncomment below  to view
          // Serial.print(HTTP_req);
          req_index = 0; // reset index
          StrClear(HTTP_req, REQ_BUF_SZ); // clear buffer
          break;
        }

        // every line of text received from the client ends with \r\n
        if (c == '\n') {
          // last character on line of received text
          // starting new line with next character read
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // a text character was received from client
          currentLineIsBlank = false;
        }
      } // end if (client.available())
    } // end while (client.connected())
    delay(1);      // give the web browser time to receive the data
    client.stop(); // close the connection
  } // end if (client)
  Relaysoff();
}

// checks if received HTTP request is switching on/off LEDs
// also saves the state of the LEDs


// sets every element of str to 0 (clears array)
void StrClear(char *str, char length)
{
  for (int i = 0; i < length; i++) {
    str[i] = 0;
  }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind)
{
  char found = 0;
  char index = 0;
  char len;

  len = strlen(str);

  if (strlen(sfind) > len) {
    return 0;
  }
  while (index < len) {
    if (str[index] == sfind[found]) {
      found++;
      if (strlen(sfind) == found) {
        return 1;
      }
    }
    else {
      found = 0;
    }
    index++;
  }

  return 0;
}

void checkIP()  // I want to use this to identify ip addresses of computers connecting.. I can log them or disallow access if unknown.. This can be bypassed but this is simple security..
{
  // client.getRemoteIP(rip); // where rip is defined as byte rip[] = {0,0,0,0 };
  if (rip[3] == 14 || rip[3] == 48 || rip[3] == 23 || rip[3] == 30 || rip[3]== 25 || rip[3] == 65) key=true;
  else key=false; // the numbers above are the last segments of the ip's of trusted computers.. 5 computers..
  //client.getRemoteIP(rip); // where rip is defined as byte rip[] = {0,0,0,0 };
  //Serial.print("IP is:");
  //Serial.println(rip[3]);
}
void Relaysoff()

{

  if(LED_state[0] == 0)   digitalWrite(5, HIGH); // door 1 open
  else LED_state[0]--; // subtracts 1 from LED+state[] kind of a timer to hold the led on slightly

  if(LED_state[1] == 0)   digitalWrite(6, HIGH); // door 1 stop
  else LED_state[1]--;

  if(LED_state[2] == 0)   digitalWrite(7, HIGH); // door 1 close
  else LED_state[2]--;

  if(LED_state[3] == 0)   digitalWrite(A0, HIGH); // door 2 open
  else LED_state[3]--;

  if(LED_state[4] == 0)   digitalWrite(A1, HIGH); // door 2 stop
  else LED_state[4]--;

  if(LED_state[5] == 0)   digitalWrite(A2, HIGH); // door 2 clos
  else LED_state[5]--;

  if(LED_state[6] == 0)   digitalWrite(A3, HIGH); // door 3 open
  else LED_state[6]--;

  if(LED_state[7] == 0)   digitalWrite(A4, HIGH); // door 2 stop
  else LED_state[7]--;

  if(LED_state[8] == 0)   digitalWrite(A5, HIGH); // door 3 close
  else LED_state[8]--;

}
void SetLEDs(void)
{
  // door 1
  if (StrContains(HTTP_req, "d1=1")) { // door 1 open
    LED_state[0] =2550;  // save LED state
    digitalWrite(5, LOW); // door 1 open
    Serial.println(F("Door1 open"));
  }
  if (StrContains(HTTP_req, "d1=2")) { // door 1 stop
    LED_state[1] = 2550;  // save LED state
    digitalWrite(6, LOW); // door 1 stop
    Serial.println(F("Door1 stop"));
  }
  if (StrContains(HTTP_req, "d1=3")) { // door 1 close
    LED_state[2] = 2550;  // save LED state
    digitalWrite(7, LOW); // door 1 close
    Serial.println(F("Door1 close"));
  }
  if (StrContains(HTTP_req, "d2=1")) { // door 2 open
    LED_state[3] = 2550;  // save LED state
    digitalWrite(A0, LOW); // door 1 open
    Serial.println(F("Door2 open"));
  }
  if (StrContains(HTTP_req, "d2=2")) { // door 2 stop
    LED_state[4] = 2550;  // save LED state
    digitalWrite(A1, LOW); // door 1 stop
    Serial.println(F("Door2 stop"));
  }
  if (StrContains(HTTP_req, "d2=3")) { // door 2 close
    LED_state[5] = 2550;  // save LED state
    digitalWrite(A2, LOW); // door 1 close
    Serial.println(F("Door2 close"));
  }

  if (StrContains(HTTP_req, "d3=1")) { // door 3 open
    LED_state[6] = 2550;  // save LED state
    digitalWrite(A3, LOW); // door 3 open
    Serial.println(F("Door3 open"));
  }
  if (StrContains(HTTP_req, "d3=2")) { // door 3 stop
    LED_state[7] = 2550;  // save LED state
    digitalWrite(A4, LOW); // door 3 stop
    Serial.println(F("Door3 stop"));
  }
  if (StrContains(HTTP_req, "d3=3")) { // door 3 close
    LED_state[8] = 2550;  // save LED state
    digitalWrite(A5, LOW); // door 3 close
    Serial.println(F("Door3 close"));
  }



}

