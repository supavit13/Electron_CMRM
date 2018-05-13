/*
 * Project Electron
 * Description:
 * Author:
 * Date:
 */
 #include "application.h"
 #include "Ublox.h"
 #include "MicroGear.h"
 #include <google-maps-device-locator.h>
 #include "HttpClient/HttpClient.h"
 #include <string>
 #include <ArduinoJson.h>
 #define APPID   "CMRM"
 #define KEY     "xjQF9lrYKUldqyz"
 #define SECRET  "9SNfMgmvHPSbSPdEZfF2w6HHs"
  #define polygon_size 25
 #define ALIAS   "Device_one"
 #define morcycid  "5a8fde523b210c00146bc0d3"

 TCPClient client;
#define N_FLOATS 4
Ublox M8_Gps;
// Altitude - Latitude - Longitude - N Satellites
float gpsArray[N_FLOATS] = {0, 0, 0, 0};
double tmplat;
double tmplng;
MicroGear microgear(client);
int r1 = D1;
int r2 = D2;
int dlc1 = D0;
int dlc2 = C5;
int br1 = C4;
int br2 = C3;
int buz1 = B4;
int buz2 = B5;
int MIL = C2;
int count = 0;
int tmppuase = 0;
int readmil;
bool statepuase = false;
bool statebooking = false;
bool statecount = false;

String str;
String latlnggoogle;
String latlnggps = "";
String code = "";
String encrypt = "";
GoogleMapsDeviceLocator locator;
/**
* Declaring the variables.
*/
unsigned int nextTime = 0;    // Next time to contact the server
HttpClient http;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
     { "Content-Type", "application/json" },
     { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;

void print_every_second()
{
    count++;
    Serial.println(count);
    if (statepuase) {
      /* code */
      tmppuase++;
    }

}
Timer timemil(357, read_every_mil);

void read_every_mil()
{
    readmil = digitalRead(MIL);
    if(readmil == 1){
      statecount = true;
    }
    if (statecount) {
      /* code */
      code+=String(readmil);
    }
    Serial.print(" ");Serial.print(code);
    if(code == "101010101010100"){
      encrypt = "0111"; //7
      digitalWrite(r1, LOW);
      digitalWrite(r2, LOW);
      timemil.stop();
    }
    if(code == "101010101010101"){
      encrypt = "1011"; //8
      digitalWrite(r1, LOW);
      digitalWrite(r2, LOW);
      timemil.stop();
    }
    if(code == "110101000000000"){
      encrypt = "1101"; //12
      digitalWrite(r1, LOW);
      digitalWrite(r2, LOW);
      timemil.stop();
    }
    if(code == "101011000000000"){
      encrypt = "1011"; //21
      digitalWrite(r1, LOW);
      digitalWrite(r2, LOW);
      timemil.stop();
    }


}

Timer timer(1000, print_every_second);
String latlng_netpie;
String lat_netpie = "13.064645426316204";
String lng_netpie = "100.89466094970705";

/* If a new message arrives, do this */
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
    Serial.print("Incoming message --> ");
    msg[msglen] = '\0';
    Serial.println((char *)msg);
    Serial.println(topic);
    if (String(topic) == "/CMRM/5a8fde523b210c00146bc0d3/Area") {
      /* code */
      latlng_netpie = String((char *)msg);
      for (size_t i = 0; i < msglen; i++) {
        /* code */
        if (latlng_netpie[i] == ',') {
          /* code */
          lat_netpie = latlng_netpie.substring(0,i);
          lng_netpie = latlng_netpie.substring(i+1);
        }
      }
    }

    if (String(topic) == "/CMRM/5a8fde523b210c00146bc0d3/startengine") {
      /* code */
      Serial.println("Instartengine");
      if (String((char *)msg) == "1") {
        //key enable
         digitalWrite(r1, HIGH);
         digitalWrite(r2, HIGH);
         //break enable
         digitalWrite(br1, HIGH);
         digitalWrite(br2, HIGH);
         delay(4000);
         //break disenable
         digitalWrite(br1, LOW);
         digitalWrite(br2, LOW);
         microgear.publish("/5a8fde523b210c00146bc0d3/ackDevice","1");
         Serial.println("ON");
         statebooking = true;
         // FreeRTOS start
         timer.start();
     }
     else if (String((char *)msg) == "0") {
        //key disenable
         digitalWrite(r1, LOW);
         digitalWrite(r2, LOW);

         microgear.publish("/5a8fde523b210c00146bc0d3/timeused",String(count)+" "+String(tmppuase));
         count = 0;
         tmppuase = 0;
         statepuase = false;
         microgear.publish("/5a8fde523b210c00146bc0d3/ackDevice","0");
         Serial.println("OFF");
         statebooking = false;
         timer.stop();
     }
    }

    if (String(topic) == "/CMRM/5a8fde523b210c00146bc0d3/outArea") {
      /* code */
      Serial.println("Area");
    }
    // if (String((char *)msg).substring(0,7) == "outArea") {
    //   /* code */
    //   Serial.println("outArea");
    //   if (String((char *)msg)[8] == '1') {
    //     /* code */
    //     digitalWrite(buz,HIGH);
    //
    //   }
    //   else{
    //     digitalWrite(buz,LOW);
    //   }
    //
    // }
    if (String(topic) == "/CMRM/5a8fde523b210c00146bc0d3/pause") {
      /* code */
      Serial.println("InPause");
      if (String((char *)msg) == "0") {
        statepuase = true;
        /* code */
        // msg = 0 stop engine
        //key disenable for pause engine
         digitalWrite(r1, LOW);
         digitalWrite(r2, LOW);
      }
      else if (String((char *)msg) == "1") {
        // msg = 1 start engine
        statepuase = false;
        //key enable
         digitalWrite(r1, HIGH);
         digitalWrite(r2, HIGH);
         //break enable
         digitalWrite(br1, HIGH);
         digitalWrite(br2, HIGH);
         delay(4000);
         //break disenable
         digitalWrite(br1, LOW);
         digitalWrite(br2, LOW);
      }

    }
    if (String((char *)msg) == "Admin") {
      /* code */
      Serial.println("Admin");
      digitalWrite(r1, LOW);
      digitalWrite(r2, LOW);
      delay(1000);
      digitalWrite(dlc1, HIGH);
      digitalWrite(dlc2, HIGH);
      delay(1000);
      digitalWrite(r1, HIGH);
      digitalWrite(r2, HIGH);
      timemil.start();
      //read input from MIL
      // int times = 0;
      // int c ;
      // int milread ;
      // int count1 = 0;
      // int count0 = 0;
      // String strsensor;
      // while(times < 7){
      //   milread = digitalRead(MIL);
      //   Serial.print(milread);
      //   if(c%100 == 0){
      //   Serial.println();
      //   }
      //   if(count0 == 150 && count1 > 31){
      //     count1 = 0;
      //     times++;
      //   }
      //   if(milread == 1){
      //     count1++;
      //     count0 = 0;
      //   }
      //   if(milread == 0)
      //   {
      //     count0++;
      //   }
      //   c++;
      // }
      //
      // if(count1 >= 214 && count1 <=218){
      //   strsensor = "0010";
      // }
      // microgear.publish("/5a8fde523b210c00146bc0d3/sendstatus",strsensor+",13.1192,100.92076");
      // digitalWrite(r1, LOW);
      // digitalWrite(r2, LOW);

    }


}

void onFoundgear(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.print("Found new member --> ");
    for (int i=0; i<msglen; i++)
        Serial.print((char)msg[i]);
    Serial.println();
}

void onLostgear(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.print("Lost member --> ");
    for (int i=0; i<msglen; i++)
        Serial.print((char)msg[i]);
    Serial.println();
}

/* When a microgear is connected, do this */
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.println("Connected to NETPIE...");
    /* Set the alias of this microgear ALIAS */
    microgear.setAlias(ALIAS);
}

// setup() runs once, when the device is first turned on.
double polygon[25][2] = {
        {
            13.445492389142101,
            100.94100952148439
        },
        {
            13.466878891069948,
            100.97671508789064
        },
        {
            13.462869067473061,
            101.00692749023438
        },
        {
            13.55489172382604,
            101.09344482421875
        },
        {
            13.57359643529671,
            101.19232177734376
        },
        {
            13.50945987010966,
            101.34613037109375
        },
        {
            13.373112476757473,
            101.55212402343751
        },
        {
            13.193872245715633,
            101.71142578125
        },
        {
            13.148372391779004,
            101.69219970703126
        },
        {
            13.033246905385807,
            101.41479492187501
        },
        {
            13.073413065643518,
            101.18957519531251
        },
        {
            13.030568929375276,
            101.18133544921876
        },
        {
            13.014500465182076,
            101.09619140625
        },
        {
            12.641958602667133,
            101.00280761718751
        },
        {
            12.518550805048974,
            101.00830078125
        },
        {
            12.483663963333774,
            100.9478759765625
        },
        {
            12.596358286049782,
            100.8819580078125
        },
        {
            12.652686907156063,
            100.70343017578126
        },
        {
            12.81623746581153,
            100.78308105468751
        },
        {
            12.920747035269438,
            100.61828613281251
        },
        {
            13.022534827559005,
            100.67596435546876
        },
        {
            13.009144078915368,
            100.84350585937501
        },
        {
            13.07876806012751,
            100.80230712890626
        },
        {
            13.180490813669216,
            100.74188232421876
        },
        {
            13.447979470764816,
            100.94512939453125
        }
    };
bool PIP(double point[], double vs[][2]){
  double x = point[0];  //lat
  double y = point[1];  //lng

    bool inside = false;
    for (int i = 0, j = polygon_size - 1; i < polygon_size; j = i++) {
        double xi = vs[i][0], yi = vs[i][1];
        double xj = vs[j][0], yj = vs[j][1];

        bool intersect = ((yi > y) != (yj > y)) && (x < (xj - xi) * (y - yi) / (yj - yi) + xi);
        if (intersect) inside = !inside;
    }

    return inside;
}


void setup() {
  pinMode(r1,OUTPUT);
  pinMode(r2,OUTPUT);
  pinMode(br1,OUTPUT);
  pinMode(br2,OUTPUT);
  pinMode(dlc1,OUTPUT);
  pinMode(dlc2,OUTPUT);
  pinMode(buz1,OUTPUT);
  pinMode(buz2,OUTPUT);
  pinMode(MIL,INPUT);

  request.hostname = "ibike.herokuapp.com";
  request.port = 80;
  request.path = "/admin/checkzone";


  // Scan for visible networks and publish to the cloud every 30 seconds
  // Pass the returned location to be handled by the locationCallback() method
  locator.withSubscribe(locationCallback).withLocatePeriodic(15);

  /* Call onMsghandler() when new message arraives */
  microgear.on(MESSAGE,onMsghandler);

  /* Call onFoundgear() when new gear appear */
  microgear.on(PRESENT,onFoundgear);

  /* Call onLostgear() when some gear goes offline */
  microgear.on(ABSENT,onLostgear);

  /* Call onConnected() when NETPIE connection is established */
  microgear.on(CONNECTED,onConnected);
  Serial.println("Starting...");
  Serial.begin(9600);
  //Config M8N.txt fixed 115200
  Serial1.begin(115200);
  /* Initial with KEY, SECRET and also set the ALIAS here */
  microgear.init(KEY,SECRET,ALIAS);

  /* connect to NETPIE to a specific APPID */
  microgear.connect(APPID);

  // subscribe is here!!!
  microgear.subscribe("/5a8fde523b210c00146bc0d3/startengine");
  microgear.subscribe("/5a8fde523b210c00146bc0d3/pause");
  microgear.subscribe("/5a8fde523b210c00146bc0d3/Area");


}
double glat;
double glng;
void locationCallback(float lat, float lon, float accuracy) {
  // Handle the returned location data for the device. This method is passed three arguments:
  // - Latitude
  // - Longitude
  // - Accuracy of estimated location (in meters)
  glat = lat;
  glng = lon;
  //Serial.print("lat: ");Serial.print(lat, 6);Serial.print(" ");Serial.print("long: ");Serial.println(lon, 6);
  latlnggoogle = String(lat) + "," + String(lon);
  microgear.publish("/5a8fde523b210c00146bc0d3/location",latlnggoogle);
  microgear.publish("/5a8fde523b210c00146bc0d3/statuslocation","GoogleGPS");
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  /* To check if the microgear is still connected */
  if(encrypt.length() == 4){
    microgear.publish("/5a8fde523b210c00146bc0d3/sendstatus",encrypt+",13.1192,100.92076");
    encrypt = "";
  }

  while(Serial1.available()){
        // delay(100);
        delayMicroseconds(50);
        char c = Serial1.read();
         if (M8_Gps.encode(c)) {
          gpsArray[0] = M8_Gps.altitude;
          gpsArray[1] = M8_Gps.latitude;
          gpsArray[2] = M8_Gps.longitude;
          gpsArray[3] = M8_Gps.sats_in_use;
        }
  }


  Serial.print(gpsArray[1]);Serial.print(" ");Serial.println(gpsArray[2]);

  if (gpsArray[1] == 0.0 && gpsArray[2] == 0.0) {
    /* code */
    //  Serial.println("INIF");
    //request every 15s
     locator.loop();
     tmplat = glat;
     tmplng = glng;
    //  request.body = "{\"data\":\"5a8fde523b210c00146bc0d3\",\"latlng\":{\"lat\":\""+glat+"\",\"lng\":\""+glng+"\"}}";
    request.body = "{\"data\":\"5a8fde523b210c00146bc0d3\",\"latlng\":{\"lat\":\"13.783894899498405\",\"lng\":\"100.73913574218751\"}}";
  }
  else{
    tmplat = gpsArray[1];
    tmplng = gpsArray[2];
    latlnggps = String(gpsArray[1])+","+String(gpsArray[2]);
    request.body =  "{\"data\":\"5a8fde523b210c00146bc0d3\"}";
    microgear.publish("/5a8fde523b210c00146bc0d3/location",latlnggps);
    microgear.publish("/5a8fde523b210c00146bc0d3/statuslocation","HardwareGPS");
  }
  //Serial.println(digitalRead(MIL));
  //****************************************//
  // Serial.print(lat_netpie.toFloat(), 6);Serial.print(" ");Serial.println(lng_netpie.toFloat(), 6);
  microgear.publish("/5a8fde523b210c00146bc0d3/testhorn",String(lat_netpie.toFloat())+","+String(lng_netpie.toFloat()));
  //13.13565375065184,101.00692749023438
  double moclatlng[2] = {lat_netpie.toFloat(),lng_netpie.toFloat()};
  if(PIP(moclatlng,polygon) && !statebooking){
    //digitalWrite(buz,LOW);
    //****************//
    digitalWrite(r1,LOW);
    digitalWrite(r2,LOW);

    digitalWrite(buz1,LOW);
    digitalWrite(buz2,LOW);

    //Serial.println("InArea");
  }
  else if(!PIP(moclatlng,polygon) && !statebooking){

    digitalWrite(r1,HIGH);
    digitalWrite(r2,HIGH);


    Serial.println("INPOST");
    //http.post(request, response, headers);
    //microgear.publish("/outArea","13.783894899498405,100.73913574218751");
    digitalWrite(buz1,HIGH);
    digitalWrite(buz2,HIGH);

  }
  else if(PIP(moclatlng,polygon) && statebooking){
    digitalWrite(buz1,LOW);
    digitalWrite(buz2,LOW);
  }
  else if(!PIP(moclatlng,polygon) && statebooking){
    digitalWrite(buz1,HIGH);
    digitalWrite(buz2,HIGH);
    //http.post(request, response, headers);
  }



  if (microgear.connected()) {
      // Serial.println("connected");

      /* Call this method regularly otherwise the connection may be lost */
      //microgear.subscribe("/5a8fde523b210c00146bc0d3/startengine");
      //delay(500);
      //microgear.subscribe("/5a8fde523b210c00146bc0d3/pause");


      // microgear.unsubscribe("/5a8fde523b210c00146bc0d3/startengine");
      // microgear.subscribe("/5a8fde523b210c00146bc0d3/outArea");
      // microgear.unsubscribe("/5a8fde523b210c00146bc0d3/outArea");
      //microgear.chat("web",latlng);        // Sent to Web
      //microgear.publish("/TestPub",latlng);  // Sent to Web
      // microgear.publish("/TestPub","Connected with Netpie");
      // microgear.chat("CMRM_Device","Connect with Netpie By Chat");

      microgear.loop();

      microgear.publish("/test","1");

  }
  else {
      Serial.println("connection lost, reconnect...");
      // microgear.on(MESSAGE,onMsghandler);
      //
      // /* Call onFoundgear() when new gear appear */
      // microgear.on(PRESENT,onFoundgear);
      //
      // /* Call onLostgear() when some gear goes offline */
      // microgear.on(ABSENT,onLostgear);
      //
      // /* Call onConnected() when NETPIE connection is established */
      // microgear.on(CONNECTED,onConnected);
      // microgear.init(KEY,SECRET,ALIAS);
      microgear.connect(APPID);
  }
  delay(100);
  //PrintDeviceID
  // Serial.println(Spark.deviceID());

  //Particle.Process() for connect cloud Particle
  // for(uint32_t ms = millis(); millis() - ms < 550; Particle.process());

}
