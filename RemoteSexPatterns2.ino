/* 

2 dual channels and 2 single channels plus a "release" channel for solenoid valves

*/

#include <ESP8266WiFi.h>

#include "./DNSServer.h"                  // Patched lib

#include <ESP8266WebServer.h>



const char WiFiAPPSK[] = "thorthor";

const byte        DNS_PORT = 53;          // Capture DNS requests on port 53

IPAddress         apIP(192, 168, 1, 1);   // Private network for server

DNSServer         dnsServer;              // Create the DNS object

ESP8266WebServer  webServer(80);          // HTTP server



String responseHTML = ""

                     

                      "<p>Ch ONE / <a href=\"http://192.168.1.1/chan-1-on\">On </a> / <a href=\"http://192.168.1.1/chan-1-pa\">A</a> / <a href=\"http://192.168.1.1/chan-1-pb\">B</a> / <a href=\"http://192.168.1.1/chan-1-pc\">C</a> / <a href=\"http://192.168.1.1/chan-1-pd\">D</a>   /   <a href=\"http://192.168.1.1/chan-1-off\"> OFF</a> </p>"
                      
                      
                     
                      "<p>Ch TWO / <a href=\"http://192.168.1.1/chan-2-on\">On</a> / <a href=\"http://192.168.1.1/chan-2-pa\">A</a> / <a href=\"http://192.168.1.1/chan-2-pb\">B</a> / <a href=\"http://192.168.1.1/chan-2-pc\">C</a> / <a href=\"http://192.168.1.1/chan-2-pd\">D</a> / <a href=\"http://192.168.1.1/chan-2-off\">Off</a><br></p>"

                     

                      "<p>Ch THREE / <a href=\"http://192.168.1.1/chan-3-on\">On</a> / <a href=\"http://192.168.1.1/chan-3-pa\">A</a> / <a href=\"http://192.168.1.1/chan-3-pb\">B</a> / <a href=\"http://192.168.1.1/chan-3-pc\">C</a> / <a href=\"http://192.168.1.1/chan-3-pd\">D</a> / <a href=\"http://192.168.1.1/chan-3-off\">Off</a><br></p>"

                      
                      "<p><a href=\"http://192.168.1.1/chan-both-on\">ALL ON </a> / <a href=\"http://192.168.1.1/chan-both-pa\">A</a> / <a href=\"http://192.168.1.1/chan-both-pb\">B</a> / <a href=\"http://192.168.1.1/chan-1-pc\">C</a> / <a href=\"http://192.168.1.1/chan-1-pd\">D</a> / <a href=\"http://192.168.1.1/chan-both-off\">ALL OFF </a><br></p>"
                      
                      
                      "<p> SPEED <a href=\"http://192.168.1.1/clkSlow\">Slower</a> / <a href=\"http://192.168.1.1/clkFast\">Faster</a> / <a href=\"http://192.168.1.1/clkReset\">Reset</a><br>"

               


                      "<p> POWER <a href=\"http://192.168.1.1/pwrLow\">Low</a> / <a href=\"http://192.168.1.1/pwrMed\">Medium</a> / <a href=\"http://192.168.1.1/pwrHigh\">High</a></p>"

                       
                       


                     
                      
                      "<p>VACUUM <br> <a href=\"http://192.168.1.1/chan-4-on\">Constant </a> / <a href=\"http://192.168.1.1/chan-4-cycle\">Cycle</a> / <a href=\"http://192.168.1.1/chan-4-off\">OFF</a><br>"

                      
                      

                      "<a href=\"http://192.168.1.1/chan-4-longer\">Longer</a> / <a href=\"http://192.168.1.1/chan-4-shorter\">Shorter</a> / <a href=\"http://192.168.1.1/chan-4-faster\">Faster</a> / <a href=\"http://192.168.1.1/chan-4-slower\">Slower</a> / <a href=\"http://192.168.1.1/chan-4-reset\">Reset</a></p>"

          

                      "<p>RELEASE <br> <a href=\"http://192.168.1.1/release-on\">Constant</a>  / <a href=\"http://192.168.1.1/release-cycle\">Cycle</a> / <a href=\"http://192.168.1.1/release-off\">OFF</a> <br> <a href=\"http://192.168.1.1/release-longer\">Longer</a> / <a href=\"http://192.168.1.1/release-shorter\">Shorter</a> / <a href=\"http://192.168.1.1/release-slower\">Slower</a> / <a href=\"http://192.168.1.1/release-faster\">Faster</a> / <a href=\"http://192.168.1.1/release-reset\">Reset</a></p>"


                     
                      "</body></html>"

                       ;




bool LEDStat = LOW;

unsigned long now = 0;

#define C1aP D0
#define C1bP D5
#define C2aP D6
#define C2bP D7
#define C3P D8
#define vacP D1
#define solP D2



byte GClk = 0;
byte Gpause = 8;
byte pauzClock = 0;
byte Ch1 = 0;
byte Ch2 = 0;
byte Ch3 = 0;
byte Ch4 = 0;
byte RelCh = 0;
byte pwr = 0;
byte pwrB = 0;
int vacOn = 100;
int vacOff = 800;
int vacClk = 0;
int solOn = 64;
int solOff = 1028;
int solClk = 0;
byte Gpwr = 40;

void setup() {

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("LetoRemote1", WiFiAPPSK);

analogWriteFreq (4000);

  pinMode(C1aP, OUTPUT);
  pinMode(C1bP, OUTPUT);
  pinMode(C2aP, OUTPUT);
  pinMode(C2bP, OUTPUT);
  pinMode(C3P, OUTPUT);
  pinMode(vacP, OUTPUT);
  pinMode(solP, OUTPUT);

  
  // if DNSServer is started with "*" for domain name, it will reply with provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  // replay to all requests with same HTML
  webServer.onNotFound([]() {
    webServer.send(200, "text/html", responseHTML);
  });


webServer.on("/clkSlow", []() {
    webServer.send(200, "text/html", responseHTML);
    Gpause = Gpause*2;
    changeStatusLED();
  });

  webServer.on("/clkFast", []() {
    webServer.send(200, "text/html", responseHTML);
    if (Gpause>1) {Gpause = Gpause/2;};
    changeStatusLED();
  });

  webServer.on("/clkReset", []() {
    webServer.send(200, "text/html", responseHTML);
    Gpause = 8;
    changeStatusLED();
  });

  webServer.on("/pwrLow", []() {
    webServer.send(200, "text/html", responseHTML);
    Gpwr = 10;
    changeStatusLED();
  });

  webServer.on("/pwrMed", []() {
    webServer.send(200, "text/html", responseHTML);
    Gpwr = 40;
    changeStatusLED();
  });

  webServer.on("/pwrHigh", []() {
    webServer.send(200, "text/html", responseHTML);
    Gpwr = 80;
    changeStatusLED();
  });

  webServer.on("/chan-1-on", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch1 = 5;
    changeStatusLED();
  });


  webServer.on("/chan-1-pa", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch1 = 1;
    changeStatusLED();
  });



  webServer.on("/chan-1-pb", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch1 = 2;
    changeStatusLED();
  });


  webServer.on("/chan-1-pc", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch1 = 3;
    changeStatusLED();
  });



  webServer.on("/chan-1-pd", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch1 = 4;
    changeStatusLED();
  });
 

  webServer.on("/chan-1-off", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch1 = 0;
    changeStatusLED();
  });

  
  webServer.on("/chan-2-on", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch2 = 5;
    changeStatusLED();
  });


  webServer.on("/chan-2-pa", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch2 = 1;
    changeStatusLED();
  });


  webServer.on("/chan-2-pb", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch2 = 2;
    changeStatusLED();
  });


  webServer.on("/chan-2-pc", []() {
   webServer.send(200, "text/html", responseHTML);
    Ch2 = 3;
    changeStatusLED();
  });



  webServer.on("/chan-2-pd", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch2 = 4;
    changeStatusLED();
  });

  
  webServer.on("/chan-2-off", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch2 = 0;
    changeStatusLED();
  });

  

  webServer.on("/chan-both-on", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch1 = 5;
    Ch2 = 5;
    Ch3 = 5;
    changeStatusLED();
  });

  
  webServer.on("/chan-both-off", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch1 = 0;
    Ch2 = 0;
    Ch3 = 0;
    changeStatusLED();
  });

webServer.on("/chan-both-pa", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch1 = 1;
    Ch2 = 1;
    Ch3 = 1;
    changeStatusLED();
  });

  webServer.on("/chan-both-pb", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch1 = 2;
    Ch2 = 2;
    Ch3 = 2;
    changeStatusLED();
  });

webServer.on("/chan-both-pc", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch1 = 3;
    Ch2 = 3;
    Ch3 = 3;
    changeStatusLED();
  });

  webServer.on("/chan-both-pd", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch1 = 4;
    Ch2 = 4;
    Ch3 = 4;
    changeStatusLED();
  });
  
    webServer.on("/chan-3-on", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch3 = 5;
    changeStatusLED();
  });


  webServer.on("/chan-3-pa", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch3 = 1;
    changeStatusLED();
  });


  webServer.on("/chan-3-pb", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch3 = 2;
    changeStatusLED();
  });


  webServer.on("/chan-3-pc", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch3 = 3;
    changeStatusLED();
  });


  webServer.on("/chan-3-pd", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch3 = 4;
    changeStatusLED();
  });

  

  webServer.on("/chan-3-off", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch3 = 0;
    changeStatusLED();
  });

    
    webServer.on("/chan-4-on", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch4 = 5;
    changeStatusLED();
  });



  webServer.on("/chan-4-cycle", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch4 = 1;
    changeStatusLED();
  });


  webServer.on("/chan-4-shorter", []() {
    webServer.send(200, "text/html", responseHTML);
    vacOn = vacOn*2;
    vacOn = vacOn/3;
    changeStatusLED();
  });


  webServer.on("/chan-4-longer", []() {
    webServer.send(200, "text/html", responseHTML);
    vacOn = vacOn*3;
    vacOn = vacOn/2;
    changeStatusLED();
  });


  webServer.on("/chan-4-faster", []() {
    webServer.send(200, "text/html", responseHTML);
    vacOff = vacOff/2;
    changeStatusLED();
  });

   webServer.on("/chan-4-slower", []() {
    webServer.send(200, "text/html", responseHTML);
    vacOff = vacOff*2;
    changeStatusLED();
  });

  webServer.on("/chan-4-reset", []() {
    webServer.send(200, "text/html", responseHTML);
    vacOff = 800;
    vacOn = 100;
    changeStatusLED();
  });

  webServer.on("/chan-4-off", []() {
    webServer.send(200, "text/html", responseHTML);
    Ch4 = 0;
    analogWrite (vacP, 0);
    changeStatusLED();
  });

   
  webServer.on("/release-on", []() {
    webServer.send(200, "text/html", responseHTML);
    digitalWrite(solP, HIGH);
    RelCh = 5;
    changeStatusLED();
  });


  
  webServer.on("/release-off", []() {
    webServer.send(200, "text/html", responseHTML);
    digitalWrite(solP, LOW);
    RelCh = 0;
  });

  webServer.on("/release-cycle", []() {
    webServer.send(200, "text/html", responseHTML);
    RelCh = 1;
    changeStatusLED();
  });


  webServer.on("/release-shorter", []() {
    webServer.send(200, "text/html", responseHTML);
    solOn = solOn*2; solOn = solOn/3;
    changeStatusLED();
  });


  webServer.on("/release-longer", []() {
    webServer.send(200, "text/html", responseHTML);
    solOn = solOn*3; solOn = solOn/2;
    changeStatusLED();
  });


  webServer.on("/release-faster", []() {
    webServer.send(200, "text/html", responseHTML);
    solOff = solOff*2; solOff = solOff/3;
    changeStatusLED();
  });

   webServer.on("/release-slower", []() {
    webServer.send(200, "text/html", responseHTML);
    solOff = solOff*3; solOff = solOff/2;
    changeStatusLED();
  });

  webServer.on("/release-reset", []() {
    webServer.send(200, "text/html", responseHTML);
    solOff = 1028;
    solOn = 64;
    changeStatusLED();
  });

  webServer.begin();

}

void changeStatusLED() {

  digitalWrite(D4, LEDStat);

  digitalWrite(D5, LEDStat);

  LEDStat = !LEDStat;

}

void loop() {

  now = millis();

  dnsServer.processNextRequest();

  webServer.handleClient();

  if (pauzClock >= Gpause) {pauzClock = 0;}
  else {++pauzClock;}

  if (pauzClock == 1) {
    ++GClk;
    ++pwr;
    --pwrB;}

  if (GClk > 160) {GClk=0;};


  dnsServer.processNextRequest();

   delay (1);

  dnsServer.processNextRequest();

 
  if ( Ch1 == 0){
    analogWrite(C1aP, 0);
    analogWrite(C1bP, 0);
  }

  if ( Ch1 == 5){
    analogWrite(C1aP, Gpwr+120);
    analogWrite(C1bP, Gpwr+120);
  }

  if (Ch1 == 1){if (GClk  == 1) {
     analogWrite(C1aP, Gpwr+140);
     }
  }

 if (Ch1 == 1){if (GClk  == 15) {
     analogWrite(C1bP, Gpwr+10);}
  }

  if (Ch1 == 1) {if (GClk == 80) {
     analogWrite(C1aP, Gpwr+10);}
  }

  if (Ch1 == 1) {if (GClk == 95) {
     analogWrite(C1bP, Gpwr+140);}
  }
  
  if ( Ch1==2) {if (GClk  == 40) {
     analogWrite(C1aP, Gpwr+175);}
  }

  if ( Ch1==2) {if (GClk  == 45) {
    analogWrite(C1bP, 0);}
  }
  
  if ( Ch1==2) {if (GClk  == 80) {
     analogWrite(C1aP,0);}
  }

   if ( Ch1==2) {if (GClk  == 85) {
     analogWrite(C1bP, Gpwr+175);}
  }
  
  if (Ch1== 2) {if (GClk==120) {
     analogWrite(C1aP, Gpwr+60);}
  }

   if (Ch1== 2) {if (GClk==125) {
     analogWrite(C1bP,0);}
  }
  
  if (Ch1== 2) {if (GClk==155) {
     analogWrite(C1aP, 0);}
  }
  
if (Ch1== 2) {if (GClk==158) {
     analogWrite(C1bP, Gpwr+60);}
  }
  
  if ( Ch1  == 3) {
    analogWrite(C1aP, pwr);
  }
  
   if ( Ch1  == 3) {
    analogWrite(C1bP, pwrB);
  }
  
   if ( Ch1  == 4) {if (GClk  == 5){
     analogWrite(C1aP, Gpwr+140);}
  }
   if ( Ch1  == 4) {if (GClk  == 15){
     analogWrite(C1bP, 0);}
  }
  
  if ( Ch1  == 4) {if (GClk  == 50) {
     analogWrite(C1aP, 0);}
  }

if ( Ch1  == 4) {if (GClk  == 65) {
     analogWrite(C1bP, Gpwr+140);}
  }
  
   if (Ch1 == 4) {if (GClk==95) {
     analogWrite(C2aP, Gpwr+90);}
  }
  
   if (Ch1  == 4) {if (GClk==110) {
     analogWrite(C2bP, Gpwr);}
  }

  if (Ch1 == 4) {if (GClk  == 130) {
     analogWrite(C1aP, Gpwr);}
  }

   if (Ch1 == 4) {if (GClk  == 145) {
     analogWrite(C1bP, Gpwr+90);}
  }

if ( Ch2 == 0){
    analogWrite(C2aP, 0);
    analogWrite(C2bP, 0);
  }

if ( Ch2 == 5){
    analogWrite(C2aP, Gpwr+120);
    analogWrite(C2bP, Gpwr+120);
  }

  if ( Ch2  == 1) {if  (GClk  == 70) {
     analogWrite(C2aP, 60);}
  }

  if ( Ch2  == 1) {if  (GClk  == 75) {
     analogWrite(C2bP, Gpwr+150);}
  }
  
 if ( Ch2  == 1){if (GClk  == 150){
     analogWrite(C2aP, Gpwr+150);}
  }
  
 if ( Ch2  == 1){if (GClk  == 155){
     analogWrite(C2bP, 60);}
  }

  if ( Ch2  == 2) {if (GClk == 1) {
     analogWrite(C2aP, Gpwr+175);
     analogWrite(C2bP, 0);}
  }
  
if ( Ch2  == 2) {if (GClk == 70) {
     analogWrite(C2aP, 80);
     analogWrite(C2bP, 0);}
  }

  if ( Ch2  == 2) {if (GClk==81){
     analogWrite(C2aP, 0);
     analogWrite(C2bP, Gpwr+175);}
  }
  
  if ( Ch2  == 2) {if (GClk==170){
     analogWrite(C2aP, 0);
     analogWrite(C2bP, 80);}
  }


  if ( Ch2  == 3) {
     analogWrite(C2aP, pwr+50);
     analogWrite(C2bP, pwrB+50);
  }
  
  if ( Ch2  == 4) {if (GClk  == 10) {
     analogWrite(C2bP, Gpwr);
     analogWrite(C2aP, Gpwr+175);}
  }

    if ( Ch2  == 4) {if (GClk==25) {
     analogWrite(C2aP, Gpwr+175);
     analogWrite(C2bP, Gpwr);}
  }
  
   if ( Ch2  == 4) {if (GClk  == 35){
     analogWrite(C2aP, 0);
     analogWrite(C2bP, Gpwr+175);}
  }
   
if ( Ch2  == 4) {if (GClk==55) {
     analogWrite(C2aP, Gpwr+175);
     analogWrite(C2bP, 0);}
  }
  
  if ( Ch2  == 4) {if (GClk  == 75){
     analogWrite(C2aP, 0);
     analogWrite(C2bP, Gpwr+175);}
  }
    
  if ( Ch2  == 4) {if (GClk==95) {
     analogWrite(C2aP, Gpwr+175);
     analogWrite(C2bP, 0);}
  }
  
  if ( Ch2  == 4) {if (GClk  == 115){
     analogWrite(C2aP, 0);
     analogWrite(C2bP, Gpwr+175);}
  }
  
  if ( Ch2  == 4) {if (GClk==135) {
     analogWrite(C2aP, Gpwr+175);
     analogWrite(C2bP, 0);}
  }
  
  if ( Ch2  == 4) {if (GClk  == 155){
     analogWrite(C2aP, 0);
     analogWrite(C2bP, 0);}
  }
  

if (Ch3==0) {
  analogWrite(C3P, 0);
}

if (Ch3==1) {if (GClk ==5){
  analogWrite(C3P, 210);}
}

if (Ch3==1) {if (GClk ==60){
  analogWrite(C3P, 0);}
}

if (Ch3==1) {if (GClk ==120){
  analogWrite(C3P, 120);}
}

if (Ch3==2) {if (GClk ==10){
  analogWrite(C3P, 255);}
}

if (Ch3==2) {if (GClk ==85){
  analogWrite(C3P, 0);}
}

if (Ch3==2) {if (GClk ==160){
  analogWrite(C3P, 80);}
}

if (Ch3==3) {if (GClk ==160){
  analogWrite(C3P, pwrB);}
}

if (Ch3==4) {if (GClk == 5){
  analogWrite(C3P, 255);}
}

if (Ch3==4) {if (GClk ==95){
  analogWrite(C3P, 0);}
}

if (Ch3==4) {if (GClk ==160){
  analogWrite(C3P, 80);}
}
if (Ch3==5) {
  analogWrite(C3P, 180);
}

if (Ch4==0) {digitalWrite(vacP,LOW);
}

if (Ch4==5) {
  digitalWrite(vacP, HIGH);
}

if (Ch4==1) {
  if (vacClk < vacOn) {digitalWrite (vacP, HIGH);};
  if (vacClk > vacOn) {digitalWrite (vacP, LOW);};
  if (vacClk > vacOff) {vacClk = 0;};
  ++vacClk;
}

if (RelCh==0) {digitalWrite(solP,LOW);
}

if (RelCh==5) {
  digitalWrite(solP, HIGH);
}

if (RelCh==1) {
  if (solClk < solOn) {digitalWrite (solP, HIGH);};
  if (solClk > solOn) {digitalWrite (solP, LOW);};
  if (solClk > solOff) {solClk = 0;};
  ++solClk;
}



}




