#include <Arduino.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SSD1306Wire.h>
#include <OLEDDisplayUi.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "string.h"
#include "font.h"
#define LED_CNT 20
#define BRIGHTNESS 80
#define LOOP_DELAY 1000
#define LED_PIN D8
#define OLED_I2C_ADDRESS 0x3C
#define SOFTSERIAL_TX D1
#define SOFTSERIAL_RX D2
#define DFPLAYER_BAUD 9600
#define SERIALBAUD 115200


const int SDA_PIN = D3;
const int SDC_PIN = D4;

CRGB leds[LED_CNT];
const char* ssid = "bluedirt";
const char* password = "gt0199f-gtd566a";
SSD1306Wire  display(OLED_I2C_ADDRESS, SDA_PIN, SDC_PIN);


String webpage = ""
"<!DOCTYPE html><html><head><title>RGB control</title><meta name='mobile-web-app-capable' content='yes' />"
"<meta name='viewport' content='width=device-width' /></head><body style='margin: 0px; padding: 0px;'>"
"<canvas id='colorspace'></canvas></body>"
"<script type='text/javascript'>"
"(function () {"
" var canvas = document.getElementById('colorspace');"
" var ctx = canvas.getContext('2d');"
" function drawCanvas() {"
" var colours = ctx.createLinearGradient(0, 0, window.innerWidth, 0);"
" for(var i=0; i <= 360; i+=10) {"
" colours.addColorStop(i/360, 'hsl(' + i + ', 100%, 50%)');"
" }"
" ctx.fillStyle = colours;"
" ctx.fillRect(0, 0, window.innerWidth, window.innerHeight);"
" var luminance = ctx.createLinearGradient(0, 0, 0, ctx.canvas.height);"
" luminance.addColorStop(0, '#ffffff');"
" luminance.addColorStop(0.05, '#ffffff');"
" luminance.addColorStop(0.5, 'rgba(0,0,0,0)');"
" luminance.addColorStop(0.95, '#000000');"
" luminance.addColorStop(1, '#000000');"
" ctx.fillStyle = luminance;"
" ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);"
" }"
" var eventLocked = false;"
" function handleEvent(clientX, clientY) {"
" if(eventLocked) {"
" return;"
" }"
" function colourCorrect(v) {"
" return v;"
" }"
" var data = ctx.getImageData(clientX, clientY, 1, 1).data;"
" var params = ["
" 'r=' + colourCorrect(data[0]),"
" 'g=' + colourCorrect(data[1]),"
" 'b=' + colourCorrect(data[2])"
" ].join('&');"
" var req = new XMLHttpRequest();"
" req.open('POST', '?' + params, true);"
" req.send();"
" eventLocked = true;"
" req.onreadystatechange = function() {"
" if(req.readyState == 4) {"
" eventLocked = false;"
" }"
" }"
" }"
" canvas.addEventListener('click', function(event) {"
" handleEvent(event.clientX, event.clientY, true);"
" }, false);"
" canvas.addEventListener('touchmove', function(event){"
" handleEvent(event.touches[0].clientX, event.touches[0].clientY);"
"}, false);"
" function resizeCanvas() {"
" canvas.width = window.innerWidth;"
" canvas.height = window.innerHeight;"
" drawCanvas();"
" }"
" window.addEventListener('resize', resizeCanvas, false);"
" resizeCanvas();"
" drawCanvas();"
" document.ontouchmove = function(e) {e.preventDefault()};"
" })();"
"</script></html>";


ESP8266WebServer server(80);   //instantiate server at port 80 (http port)
SoftwareSerial mySoftwareSerial(SOFTSERIAL_RX, SOFTSERIAL_TX); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

void setColor(CRGB color){
    for ( int i=0;i<LED_CNT;i++){
        leds[i] = color;
    }
}

void drawTextAlignmentDemo() {
    // Text alignment demo
  display.setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text


  // The coordinates define the center of the text
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 22, "Center aligned (64,22)");

  // The coordinates define the right end of the text
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 33, "Right aligned (128,33)");
}

void setup() {
    // put your setup code here, to run once:
    display.init();
    display.clear();
    display.display();


    mySoftwareSerial.begin(DFPLAYER_BAUD);
    Serial.begin(SERIALBAUD);

    display.flipScreenVertically();


    pinMode(D5,INPUT_PULLUP);
    pinMode(D6,INPUT_PULLUP);
    pinMode(A0,INPUT);
    pinMode(D0,OUTPUT);
    pinMode(LED_PIN,OUTPUT);
    FastLED.addLeds<NEOPIXEL,LED_PIN>(leds, LED_CNT);    
    FastLED.setBrightness(BRIGHTNESS);
    
    if (!myDFPlayer.begin(mySoftwareSerial,false)) {  //Use softwareSerial to communicate with mp3.
      
        Serial.println(F("Unable to begin:"));
        Serial.println(F("1.Please recheck the connection!"));
        Serial.println(F("2.Please insert the SD card!"));

        while(true){
            delay(0); 
        }
    }

    Serial.println(F("DFPlayer Mini online."));  
    delay(1000);

    Serial.print("Connecting to Wifi...");
    display.drawString(64, 10, "Connecting to WiFi");
    WiFi.begin(ssid, password); //begin WiFi connection
    Serial.println("");
  
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.on("/", [](){
        server.send(200, "text/html", webpage);

        String red = server.arg(0); // read RGB arguments
        String green = server.arg(1);
        String blue = server.arg(2);

        CRGB c;
        c.b = blue.toInt();
        c.r = red.toInt();
        c.g = green.toInt();
        setColor(c);
    });
    // server.on("/blue", [](){
    //     setColor(CRGB::Blue);
    //     server.send(200, "text/plain", "OK: Blue");
    // });
    // server.on("/red", [](){
    //     setColor(CRGB::Red);
    //     server.send(200, "text/plain", "OK: Red");
    // });
    server.begin();
    Serial.println("Web server started!");
    myDFPlayer.volume(10);
    myDFPlayer.play(23); 
}


void handleInputPins(){
    int d5 = digitalRead(D5);
    int d6 = digitalRead(D6);
    if ( d5 == 0 || d6 == 0){
        digitalWrite(D0,1);
    }
    else{
        digitalWrite(D0,0);
    }
}
void handleAnalog(){
    
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    String stringOne =  String(analogRead(A0), DEC); 
    display.drawString(0, 10, stringOne );
}
int i = 0;
void loop() {
   server.handleClient();
   display.clear();
   handleInputPins();
   handleAnalog();
   drawTextAlignmentDemo();
   i++;
   if ( i == 1000){
        Serial.print("Refresh Rate: ");
        Serial.print(FastLED.getFPS() );
        i = 0;
   }
   FastLED.show();
   display.display();
}

