// The MIT License
// 
// Copyright (c) 2015 Neil Webber
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include <Ethernet.h>
#include <EEPROM.h>

//
// Ethernet MAC address to use.
//     Usually Arduino Ethernet shields come with an address printed on
//     them (sometimes on underside of the board). Use that one here.
//
byte mac[] = { 0x66, 0x55, 0x44, 0x33, 0x22, 0x11 };  // REPLACE WITH REAL ADDR

//
// IFTTT Maker parameters:
//     Key -- Obtained when setting up/connecting the Maker channel in IFTTT
//   Event -- Arbitrary name for the event; used in the IFTTT recipe.
//
char MakerIFTTT_Key[] = "ajJ1jaAxQOlnKl5KmZeGRd";
char MakerIFTTT_Event[] = "garage_door";
//
// The IFTTT/Maker channel allows for three values to be reported
// which can be used as "ingredients" in the IFTTT recipe.
//
// In this simple example the values come from:
//    reading a PIN                    (reported as value1)
//    reporting uptime in milliseconds (reported as value2)
//    the string "via #GarageDoor"     (reported as value3)
//
//
#define READ_THIS_PIN       3      // will be reported as "value1"
byte doorPin=4;
// helper functions for constructing the POST data
// append a string or int to a buffer, return the resulting end of string

char *append_str(char *here, char *s) {
    while (*here++ = *s++)
  ;
    return here-1;
}

char *append_ul(char *here, unsigned long u) {
    char buf[20];       // we "just know" this is big enough

    return append_str(here, ultoa(u, buf, 10));
}


//
// This is called once per iteration to read the pin
// and send a POST to trigger the IFTTT/Maker event
//

void update_event() {
    EthernetClient client = EthernetClient();

    // connect to the Maker event server
    client.connect("maker.ifttt.com", 80);

    // construct the POST request
    char post_rqst[256];    // hand-calculated to be big enough

    char *p = post_rqst;
    p = append_str(p, "POST /trigger/");
    p = append_str(p, MakerIFTTT_Event);
    p = append_str(p, "/with/key/");
    p = append_str(p, MakerIFTTT_Key);
    p = append_str(p, " HTTP/1.1\r\n");
    p = append_str(p, "Host: maker.ifttt.com\r\n");
    p = append_str(p, "Content-Type: application/json\r\n");
    p = append_str(p, "Content-Length: ");

    // we need to remember where the content length will go, which is:
    char *content_length_here = p;

    // it's always two digits, so reserve space for them (the NN)
    p = append_str(p, "NN\r\n");

    // end of headers
    p = append_str(p, "\r\n");

    // construct the JSON; remember where we started so we will know len
    char *json_start = p;

    // As described - this example reports a pin, uptime, and "via#tuinbot"
    p = append_str(p, "{\"value1\":\"");
    p = append_ul(p, analogRead(READ_THIS_PIN));
    p = append_str(p, "\",\"value2\":\"");
    p = append_ul(p, millis()/1000);
    p = append_str(p, "\",\"value3\":\"");
    p = append_str(p, "via #GarageDoor");
    p = append_str(p, "\"}");

    // go back and fill in the JSON length
    // we just know this is at most 2 digits (and need to fill in both)
    int i = strlen(json_start);
    content_length_here[0] = '0' + (i/10);
    content_length_here[1] = '0' + (i%10);

    // finally we are ready to send the POST to the server!
    client.print(post_rqst);
    client.stop();
}

// called once at system reset/startup time
void setup() {
    // this sets up the network connection, including IP addr via DHCP
    Ethernet.begin(mac);

    // the input pin for this example
    pinMode(READ_THIS_PIN, INPUT_PULLUP);
    pinMode(doorPin,INPUT_PULLUP);
}

// how often to read the pins and update IFTTT
#define LOOP_DELAY_MSEC     (120*1000L)   // 5 minutes

// main body; called over and over if it ever returns
void loop() {

    // DHCP lease check/renewal (library only sends request if expired)
    Ethernet.maintain();
    
if (digitalRead(doorPin)==HIGH) {
    // read the pins, send to IFTTT/Maker
    update_event();
    // only "this often"
    delay(LOOP_DELAY_MSEC);
    }
    
}
