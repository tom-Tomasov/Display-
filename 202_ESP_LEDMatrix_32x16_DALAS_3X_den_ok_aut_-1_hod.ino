
//  27.9.2024-31.12.2025
/*
    pouzite piny 
      pinCidlaDS 5 =(senzoryDS)  Dalas 3x I2s= teplotni čidla
    // DATA_PIN 15   for  MD_Parola P = MD_Parola= nepouzité
      DIN_PIN 15 =display  (Led matrix 32x16)
      CS_PIN  13 =display  (Led matrix 32x16)
      CLK_PIN 12 =display  (Led matrix 32x16)
      analogRead(A0) =fotosenzor
      buzzer = D2; GPIO 4
    
*/ 
      #include "note.h" 
      #include <SolarCalculator.h>
    // #include <Timezone.h>
      #include <ESP8266WiFi.h>
      #include <ESP8266HTTPClient.h>
      #include <NTPClient.h>
      #include <WiFiUdp.h>
      #include "fonts.h"   
      #include <OneWire.h>
      #include <DallasTemperature.h> //<DS18B20Events.h>
      const int pinCidlaDS = 5;
      OneWire oneWireDS(pinCidlaDS);// vytvoření instance oneWireDS z knihovny OneWire
      DallasTemperature senzoryDS(&oneWireDS);// vytvoření instance senzoryDS z knihovny DallasTemperature

      #define TIMEDHT 3000   //4500 zpozdeni testu čidel dalas
      #define NUM_MAX 16         
      #define LINE_WIDTH 64    //32 puvodně ted 64.  // 128 nefunguje

     // for D1 mini
      #define DIN_PIN 15  
  //  #define DATA_PIN 15    //for  MD_Parola P = MD_Parola
      #define CS_PIN  13  
      #define CLK_PIN 12  
      #define MAX_DEVICES 16
      #define ROTATE  90     
      #define DEBUG(x)    // nechat 
      #include "max7219.h"   //nehybat nepojede program ~~
     int color = 0;  // klopny obvod pro svetlo zap a vyp 
     int zrcadlo = 0;  // klopny obvod pro zrcadlo zap 
    // =======================================================================
            const char* ssid     = "TP-Link1";     // SSID of local network
            const char* password = "marketa12";   // Password 
    // =======================================================================
     long utcOffset = 1;   //  nejede nechat 1  
     long localEpoc = 0;   // 2 časový posun (utcOffset) 0 PŘIDÁNO NA 1 OPOŽDUJE SE DEN O 1 HOD
     long localMillisAtUpdate = 0; 
     //=============================================================

    WiFiUDP ntpUDP;
    HTTPClient http;
    WiFiClient client;
 NTPClient ntpClient(ntpUDP, "cz.pool.ntp.org", utcOffset * 3600, 60000*60);  //utcOffset * 3600// Aktualizace každou minutu(hodinu*60)
   
      
       // Location  Karvina
  double transit, sunrise, sunset;
  double latitude = 49.8672914;
  double longitude = 18.5555467;

   
      uint16_t scrollPause = 0; // in milliseconds 1000
      uint8_t degC[] = { 6, 3, 3, 56, 68, 68, 68 }; // Deg C
      
        char str[10]; //oprava puvodně 0 
        char mereni1[6];  //oprava puvodne 2  // [2]fotorezistor
       // int h, m, s, day, month, year, dayOfWeek;
      byte h, m, s, day, month, dayOfWeek;  // upraveno GPT
       int year; 

        String date;
        String buf="";
        
        int clockOnly = 0;
        float teplota  = .02f;
        float teplota2 = .02f;
        float teplota3 = .02f;
        uint32_t timerDHT = TIMEDHT;
        int soucet;
        int mi;
        int prumer;
        //------------------------------
        #define MAX_DIGITS 16
        byte dig[MAX_DIGITS]={0};
        byte digold[MAX_DIGITS]={0};
        byte digtrans[MAX_DIGITS]={0};
        long dotTime = 0;
        long clkTime = 0;
        int dx=0, dy=0;
        int xPos=0, yPos=0;
      //  int fan1 = 0;
      //  int fan2 = 0;
        byte del=0;

//*************** nastaveni  void setup() *******************
          void setup() 
    {
      buf.reserve(500);
      Serial.begin(115200);
      initMAX7219();
     
      sendCmdAll(CMD_SHUTDOWN, 1);      // zapni display
      sendCmdAll(CMD_INTENSITY, 0); 
      sendCmd(12,10,1);
      sendCmd(13,10,1);
      sendCmd(14,10,1);
      sendCmd(15,10,1);  //  nastaveni jasu  0= min ,15 max
      DEBUG(Serial.print("Connecting to WiFi ");)
      WiFi.begin(ssid, password);
      clr();
      xPos=0;
      yPos=0;
   
       printString("WI-FI PRIPOJENI..", font3x7);
      refreshAll();
      while (WiFi.status() != WL_CONNECTED)  {
      delay(500); DEBUG(Serial.print("."));  }
                                                
      clr();
      xPos=10;
      DEBUG(Serial.println(""); Serial.print(" MyIP: "); Serial.println(WiFi.localIP());)
      printString((WiFi.localIP().toString()).c_str(), font3x7);
      refreshAll();
       getNtpTime();
      //getTime();

      senzoryDS.begin();
   
    }
    // -----------end---void setup() -------nesahat------
        unsigned int curTime,updTime=0;
        int dots,mode;
        
 // ================ Hlavni smyčka void loop() ==== Nasteveni a zapnutí 4x Display ===================
  void loop()
    {
      curTime = millis();
      if(curTime-updTime> 3600000) {    // 3600000 jedna hodina     600000 10 min
        updTime = curTime;
       getNtpTime();                }      //  getTime();/////update time every 600s=10m
                                       
      dots = (curTime % 1000)<500;     // blikani 2 times/sec
      mode = (curTime % 60000)/15000;  // Změna displeje každých 20s = 20000 milisec // 4x 15 sec = 15000milis
      updateTime();
      // nezpomenout upravit Změna displeje na 15000 (15sec) pro jen 3x display
     
     if(mode==0) drawTime0();     //  zobrazeni velky Čas a zobrazení teploty Venku
     if(mode==1) drawTime2();     //  zobrazeni maly  Čas a vychod, zapad, luminisence + den a datum
     if(mode==2) drawTime3();     //  zobrazeni maly  Čas a teploty Radiator
     if(mode==3) drawTime1();     //  zobrazení velky Čas a teploty Balkon
     
     
       refreshAll(); 
      calcSunriseSunset(year, month, day, latitude, longitude, transit, sunrise, sunset);
    
    /* 
    //-********************GPT pridano kontrola čidel *******
           int count = senzoryDS.getDeviceCount();
      if (count > 0) teplota = senzoryDS.getTempCByIndex(0);
      if (count > 1) teplota2 = senzoryDS.getTempCByIndex(1);
      if (count > 2) teplota3 = senzoryDS.getTempCByIndex(2);
    //--------------------------------------------------------
  */ 
      }
// ==============konec hlavni smyčky-----------------==============================================
        
        char* monthNames[] = {"LEDEN","UNOR","BREZEN","DUBEN","KVETEN","CERVEN","CERVENEC","SRPEN","ZARI","RIJEN","LISTOPAD","PROSINEC"};
        char txt[10];
        static const char weekNames[7][10] PROGMEM ={"PONDELI"," UTERY"," STREDA","CTVRTEK","PATECEK","SOBOTKA"," NEDELE"};
        char txt1[10];
        char buffer[10];
        char buffer1[10];
       
      
//============== Display 0 ===========================

         void drawTime0()
      {
           clr();
          yPos = 0;     //0 = nahoru
          xPos = 0;         
         // xPos = (h>9) ? 0 : 2;
          sprintf(txt,"%02d",h); //GPT
          printString(txt, digits7x16);
          
          if(dots) printCharX(':', digits5x16rn, xPos);
          xPos +=2;
          //xPos+=(h>=22 || h==20)?1:2;
          sprintf(txt,"%02d",m);
          printString(txt, digits7x16);
        //--------------------------------------
            // Wait for a time TIMEDHT = 3000 puvodně 4500
          if ((millis() - timerDHT) > TIMEDHT) {
            // Update the timer
            timerDHT = millis(); // puvodne konec smyčky } 
            senzoryDS.requestTemperatures();
            teplota = (senzoryDS.getTempCByIndex(0)); }
            
          //-------------------------------------
            yPos = 0;               //    y0 = nahoru
            xPos = 33;            //x vice je doprava 
          //   sendCmd(11,10,0);
        //(pozice,jas,intesita jasu) // intesita nastaveni jasu  0= min ,15 max 
            if (teplota > 70 )                  { printString("  HORII", font3x7);  
              sendCmd(12,10,15);sendCmd(13,10,15);sendCmd(14,10,15);sendCmd(15,10,15); }
            if (teplota > 40 && teplota <=70)    { printString("  SAUNA", font3x7);  
              sendCmd(12,10,12);sendCmd(13,10,12);sendCmd(14,10,12);sendCmd(15,10,12); }
            if (teplota <= 40 && teplota > 35)  { printString("MALO HYC", font3x7);
              sendCmd(12,10,10);sendCmd(13,10,10);sendCmd(14,10,10);sendCmd(15,10,10); } 
            if (teplota <= 35 && teplota > 30)  { printString("BUDE HIC", font3x7);
              sendCmd(12,10,5);sendCmd(13,10,5);sendCmd(14,10,5); sendCmd(15,10,5); } 
            if (teplota <= 30 && teplota > 25)  { printString("  TEPLO", font3x7);
              sendCmd(12,10,2);sendCmd(13,10,2);sendCmd(14,10,2); sendCmd(15,10,2); }
            if (teplota <= 25 && teplota > 20)  { printString(" TEPLEJI", font3x7); } 
            if (teplota <= 20 && teplota > 15)  { printString(" CHLADNO", font3x7); 
              sendCmd(12,10,1);sendCmd(13,10,1);sendCmd(14,10,1); sendCmd(15,10,1); }
            if (teplota <= 15 && teplota > 10)  { printString("  ZIMA", font3x7); }     
            if (teplota <= 10 && teplota > 5)   { printString("  KOSA", font3x7); }  
            if (teplota <= 5 && teplota >= 0)   { printString(" MRAZIK", font3x7);} 
            if (teplota < 0 && teplota > -5)    { printString("   LEDIK", font3x7);}
            if (teplota <= -5 && teplota >-10)  { printString(" NANUK", font3x7); }
            if (teplota <= -10 && teplota >-15) { printString(" MRAZAK", font3x7); }
            if (teplota <= -15 && teplota >-25) { printString("SARKOFAG", font3x7);} 
            if (teplota <= -127)                { printString("  ERROR", font3x7); 
              sendCmd(12,10,1);sendCmd(13,10,1);sendCmd(14,10,1); sendCmd(15,10,1);}
                         
          //  for(int i=0;i<32;i++) scr[32+i]<<=1;   
          //  <<=1    vyška řadku :-) <<=1 je dole  vice je dolu       

            yPos = 1;                 //    y0 = nahoru   y1 = dolu
            xPos = 41;                //x vice je doprava 
              sprintf(txt,"%.1f",teplota);
              printString(txt,font3x7 ); 
              printString("'C", font3x7); 
     //  ZAJIMAVE NEJEDE TEPLOTA POKUD ODKOMENTUJI a hazi error
     //  for(int i=64;i<96;i++) scr[96+i]<<=3;    //  <<=1    vyška řadku :-) <<=1 je dole  vice je dolu
     // for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;
//------------end display 0   ---------------------------------------
       }
//--------------------------------------------------------


//============== Display 1 ==========================
    void drawTime1()
    {
       clr();
        yPos = 0;
        xPos = 0;
       // xPos = (h>9) ? 0 : 1;   //GPT  //x vice je doprava 
        sprintf(txt,"%02d",h); // gpt
        //  sprintf(txt,"%d",h); sprintf(txt, (h < 10) ? "0%d" : "%d", h);

        printString(txt, digits5x16rn);
        if(dots) printCharX(':', digits5x16rn, xPos);
        xPos += 2;
        //xPos+=(h>=22 || h==20) ? 1 : 2;    //GPT   //x vice je doprava 
        sprintf(txt,"%02d",m);
        printString(txt, digits5x16rn);

        sprintf(txt,"%02d",s);
        printString(txt, font3x7);

       yPos = 0;
       xPos = 36;  //   37            //x vice je doprava 
        printString("BALKON", font3x7);  
   //-----------------------------------------
    // Wait for a time TIMEDHT = 4500
  if ((millis() - timerDHT) > TIMEDHT) {
    // Update the timer
    timerDHT = millis();// puvodne konec smyčky }
    senzoryDS.requestTemperatures();
    teplota2 = (senzoryDS.getTempCByIndex(1));   }

    yPos = 1;
    xPos = 41;                //x vice je doprava           
        sprintf(txt,"%.01f",teplota2);
        printString(txt, font3x7);    
        printString("'C", font3x7);   
      // for(int i=64;i<128;i++) scr[128+i]<<=3;    //  <<=1    vyška řadku :-) <<=1 je dole  vice je dolu
       //  for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;    //  <<=1    vyška řadku :-) <<=1 je dole  vice je dolu
 
 //*******************automaticke zrcadlo *************************************************************
  //--------------denne zrcadlo zap 10 hod ------------------------
      if ( zrcadlo == 0 && h == 10 && m == 0) { zrcadlo = 1;  // printString("zrcadlo:", font3x7); 
          http.begin(client,"http://192.168.0.200/5/off"); http.GET();  http.end();} 
            //  http.begin(client,"http://192.168.0.210/16/on"); http.GET();  http.end();} // zapni cool
      if ( zrcadlo == 1 && h == 10 && m == 1) {  zrcadlo = 0; } 

    // ----------sobota zrcadlo zap 7hod ---------------------
      if ((dayOfWeek-1) == 5 && h == 7 && m == 0) { zrcadlo = 1;  // sobota (weekNames-1)==5
          http.begin(client,"http://192.168.0.200/5/off"); http.GET();  http.end();} // zapni cool(zrcadlo) 
      if ( zrcadlo == 1 && h == 7 && m == 1) {  zrcadlo = 0; } 
    // ---------------nedele zrcadlo zap  7hod ---------------
      if ((dayOfWeek-1) == 6 && h == 7 && m == 0) { zrcadlo = 1;  // nedele  (weekNames-1)==6
          http.begin(client,"http://192.168.0.200/5/off"); http.GET();  http.end();} // zapni cool(zrcadlo) 
      if ( zrcadlo == 1 && h == 7 && m == 1) {  zrcadlo = 0; }
     // Serial.println (dayOfWeek-1); //sobota ==5

//---------------nesahat-----------------------------
       }
//---------------END DISPAY 1-----------------------------

//============== Display 2 ==========================
        void drawTime2()
      {
        clr();
        yPos = 0;  
        xPos = 0;                 // pozice nahoru=0 , dolu=1
       // xPos = (h>9) ? 0 : 2;      //32 pozice x na display  ?1:2
        sprintf(txt,"%02d",h);
        printString(txt, digits5x8rn);
        if(dots) printCharX(':', digits5x8rn, xPos);
        xPos +=2; //POSUN O DVě ČÍSLA
        //xPos+=(h>=22 || h==20)?1:2;   //xPos+=(h>=22 || h==20)?1:2;
        sprintf(txt,"%02d",m);
        printString(txt, digits5x8rn);
        sprintf(txt,"%02d",s);
        printString(txt, digits3x5);
        
        //------------- FOTO ODPOR na pin A0 -------------------------
     
        yPos = 0;
        xPos = 34;      //x vice je doprava 
    
 // int mereni = analogRead(A0); //cteni hodnoty fotorezistoru

       // Wait for a time TIMEDHT = 4500
          if ((millis() - timerDHT) > TIMEDHT) {
            // Update the timer
            timerDHT = millis(); // puvodne konec smyčky  
            int mereni = analogRead(A0); 
            prumer = mereni /15 ;            } // prumer = (mereni / 15);
                                       
  //--------konec smyčky foto odporu  (mereni) 

  //---------------------zapnout a vypnout svetla  podle foto odporu  *********************************--------

    if (color == 1 && prumer > 23 && prumer < 26 && h >=4 && h <=8) {  color = 0;// printString("VYCHOD:", font3x7); 
     http.begin(client,"http://192.168.0.210/4/off"); http.GET();  http.end(); // vypni color
     http.begin(client,"http://192.168.0.210/13/off"); http.GET();  http.end(); // vypni color
        sendCmd(12,10,1);sendCmd(13,10,1);sendCmd(14,10,1); sendCmd(15,10,1);    }  
    if (color == 0 && prumer < 26 && prumer > 20 && h >=15 && h <=22) {  color = 1; //printString("ZAPAD:", font3x7); 
     http.begin(client,"http://192.168.0.210/4/on"); http.GET();  http.end(); // zapni color
     http.begin(client,"http://192.168.0.210/13/on"); http.GET();  http.end(); // zapni color
       sendCmd(12,10,0);sendCmd(13,10,0);sendCmd(14,10,0); sendCmd(15,10,0);   }   
    
   xPos = 53;  
   yPos = 0;  //54
   if (color == 0) {printString(")", font3x7); sprintf(mereni1,"%d",prumer); printString(mereni1,font3x7); }// ")" změněno na "|" s mezerama
   if (color == 1) {printString("/", font3x7); sprintf(mereni1,"%d",prumer); printString(mereni1,font3x7); }// "/" zmeněno na "|"
  
   for(int i=0;i<32;i++) scr[32+i]<<=1;    //  <<=1    vyška řadku :-) <<=1 je dole  vice je dolu
  // for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;    //  <<=1    vyška řadku :-) <<=1 je dole  vice je dolu   
                                      
   

    //-------------------------- slunce ok--------------
    
      xPos = 35;      //34
      yPos = 0;
      char txt2[4];
      char txt3[4];
      char str[6];
      if (h < 8) { sprintf(txt2,"%.5s",(hoursToString(sunrise + utcOffset, str))); 
            printString(txt2,digits3x5);}     //font3x7
      if (h >= 8) {sprintf(txt3,"%.5s",(hoursToString(sunset + utcOffset, str)));  
            printString(txt3,digits3x5);}     //font3x7
      
   //   for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=0;    //  <<=1    vyška řadku :-) <<=1 je dole  vice je dolu
      //------------- DEN MESIC ROK  V PRAVO DOLE ZONA (2) -------------------------
        yPos = 1;       // pozice nahoru=0 , dolu=1
        xPos = 35;        //32     --33     //x vice je doprava ;'][\=--0-]
  //  const  char buffer = ("%02d.%02d.%2d",day,month,year-2000);
            //   sprintf(buffer1,"%02d",day);printString(buffer1,digits3x5); 
            //   sprintf(buffer,"%02d.%2d",month,year-2000);    
          //  sendCmd(12,12,0);delay (50);
    // sprintf(buffer1,"%02d.",day);   printString(buffer1,digits3x5); //dodano
   //  sprintf(buffer,"%02d.%02d",month,year-2000);  
     sprintf(buffer,"%02d.%02d.%2d",day,month,year-2000);
      printString(buffer,font3x7); 
          //  sendCmd(12,12,1);
                         //  P.displayZoneText(3, buffer, PA_LEFT, 35, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
          // printString(txt, font3x7);
   //   for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;    //  <<=1    vyška řadku :-) <<=1 je dole  vice je dolu
          
    //--------------automat  ++ DEN V TYDNU V LEVO DOLE ---------------------------------------
       yPos = 1;
       xPos = 1;
     //-*-*-*-*-*-*-*- automat *-*-*-*-*-*-*--*-*-*-*-*-*-*-*--*-*-*-*-*-*-*-*-*-*-*--*-*-       
      if        (month==12 && day==31 && h==12 && m<=10) { printString("SILVESTR", font3x7);}
      else if   (month==12 && day==31 && h==23 && m>=55) { printString("SILVESTR", font3x7);}
      else if   (month==1  && day==1  && h==0)  { printString("NOVY ROK", font3x7);}
  //*-*-**-*-*-*-*--*-*-*-*-*-*--*-*-**--*--*-*-*-*-*-*---*--*--*-*-*-*-*---*-**-*-*-*--*  
      else      {sprintf(txt,"%s",weekNames[dayOfWeek-1]), printString(txt, font3x7);}
      
        for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1; 
  
  //----------------------------------------------------------
      }      // nesahat------------
 //***************END DISPAY 2*********************************************  

//-------------drawTime3------//void showAnimClock()----------------
        void drawTime3()
     {
        clr();
        //  yPos = 0;     //0 = nahoru dodano
        //  xPos = (h>9) ? 1 : 0;   // dodano 0:2
          sprintf(txt,"%02d",h);
        byte digPos[4]={1,8,17,25};
          int digHt = 12;
          int num = 4; 
          int i;

          if(del==0) {
            del = digHt;
            for(i=0; i<num; i++) digold[i] = dig[i];

            dig[0] = h/10 ; // GPT  ? h/10 : 10;
            dig[1] = h%10;
            dig[2] = m/10;
            dig[3] = m%10;

            for(i=0; i<num; i++)  digtrans[i] = (dig[i]==digold[i]) ? 0 : digHt;
          } else
            del--;
          
          clr();
          for(i=0; i<num; i++) {
            if(digtrans[i]==0) {
              dy=0;
              showDigit(dig[i], digPos[i], dig6x8);
            } else {
              dy = digHt-digtrans[i];
              showDigit(digold[i], digPos[i], dig6x8);
              dy = -digtrans[i];
              showDigit(dig[i], digPos[i], dig6x8);
              digtrans[i]--;
            }
          }
          dy=0;
          setCol(15,dots ? B00100100 : 0);
   //-------------- teplota radiator-----------------
        // Wait for a time TIMEDHT = 6000
          if ((millis() - timerDHT) > TIMEDHT) {
            timerDHT = millis(); // puvodne konec smyčky } 
            senzoryDS.requestTemperatures();
            teplota3 = (senzoryDS.getTempCByIndex(2));   }
        //-------------------------------------
            yPos = 1;               //    y0 = nahoru
            xPos = 0;            //x vice je doprava 
            if (teplota3 > 45)                   { printString("TOPI HURA", font3x7); } 
            if (teplota3 <= 45 && teplota3 > 40) { printString("TOPI MINI", font3x7); } 
            if (teplota3 <= 40 && teplota3 > 35) { printString("TEPLO ", font3x7);
             printString("!",digits5x8rn); }  // srdce
            if (teplota3 <= 35 && teplota3 > 30) { printString(" VICE ", font3x7); 
            printString("(#)",digits5x8rn);}    //$ nahoru
            if (teplota3 <= 30 && teplota3 > 25) { printString("MALO ", font3x7);
            printString("($)",digits5x8rn);}    // rovne
            if (teplota3 <= 25 && teplota3 > 20) { printString("NETOPI", font3x7); 
            printString("(%)",digits5x8rn);}    // dolu
         //   sendCmd(11,10,3); sendCmd(12,10,3); sendCmd(13,10,1);sendCmd(14,10,1); sendCmd(15,10,1); }  
            if (teplota3 <= 20 && teplota3 > 15) { printString(" CHLADNO", font3x7); }
            if (teplota3 <= 15 && teplota3 > 10) { printString("    ZIMA", font3x7); }       
            if (teplota3 <= 10 && teplota3 > 0 ) { printString("ZMRZNEM ", font3x7); }  
            if (teplota3 <= -127)                { printString("ERR.CIDLO ", font3x7); }  
           
            yPos = 0;   //    y0 = nahoru   y1 = dolu 
            xPos = 34;   //x vice je doprava
            printString("RADIATOR", font3x7);    
             
            yPos = 1;
            xPos = 41;                
            sprintf(txt,"%.1f",teplota3);
            printString(txt,font3x7 ); 
            printString("'C", font3x7); 
           for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;

 //***** cas a hodina automat  hraj pisnicku  ********************************
      if   (month==1 && day==1 && h== 0 && m== 1 ) { hraj();}
      }   // nesahat
      
      //========== čteni fontu ======================================
      int charWidth(char c, const uint8_t *font)
      {
        int fwd = pgm_read_byte(font);
        int fht = pgm_read_byte(font+1);
        int offs = pgm_read_byte(font+2);
        int last = pgm_read_byte(font+3);
        if(c<offs || c>last) return 0;
        c -= offs;
        int len = pgm_read_byte(font+4);
        return pgm_read_byte(font + 5 + c * len);
      }

// =======================================================================

      int stringWidth(const char *s, const uint8_t *font)
      {
        int wd=0;
        while(*s) wd += 1+charWidth(*s++, font);
        return wd-1;
      }

// =======================================================================

      int stringWidth(String str, const uint8_t *font)
      {
        return stringWidth(str.c_str(), font);
      }

// =======================================================================

      int printCharX(char ch, const uint8_t *font, int x)
      {
        int fwd = pgm_read_byte(font);
        int fht = pgm_read_byte(font+1);
        int offs = pgm_read_byte(font+2);
        int last = pgm_read_byte(font+3);
        if(ch<offs || ch>last) return 0;
        ch -= offs;
        int fht8 = (fht+7)/8;
        font+=4+ch*(fht8*fwd+1);
        int j,i,w = pgm_read_byte(font);
        for(j = 0; j < fht8; j++) {
          for(i = 0; i < w; i++) scr[x+LINE_WIDTH*(j+yPos)+i] = pgm_read_byte(font+1+fht8*i+j);
          if(x+i<LINE_WIDTH) scr[x+LINE_WIDTH*(j+yPos)+i]=0;
        }
        return w;
      }

// =======================================================================

        void printChar(unsigned char c, const uint8_t *font)
        {
          if(xPos>NUM_MAX*8) return;
          int w = printCharX(c, font, xPos);
          xPos+=w+1;
        }

// =======================================================================

          void printString(const char *s, const uint8_t *font)
          {
            while(*s) printChar(*s++, font);
            //refreshAll();
          }

          void printString(String str, const uint8_t *font)
          {
            printString(str.c_str(), font);
          }

// =================time ======================================================
void getNtpTime()
{
 ntpClient.begin();

  if (!ntpClient.update()) {
    DEBUG(Serial.println("NTP update selhalo"));
    return;
  }

  // Uložení hodnot do tvých proměnných
  time_t rawTime = ntpClient.getEpochTime();
  struct tm* timeinfo = gmtime(&rawTime);

  h = timeinfo->tm_hour;
  m = timeinfo->tm_min;
  s = timeinfo->tm_sec;
  day = timeinfo->tm_mday;
  month = timeinfo->tm_mon + 1;
  year = timeinfo->tm_year + 1900;
  dayOfWeek = (timeinfo->tm_wday == 0) ? 7 : timeinfo->tm_wday; // 1 = pondělí, 7 = neděle

  localMillisAtUpdate = millis();
  localEpoc = h * 3600 + m * 60 + s;

  DEBUG(Serial.println(String(h) + ":" + String(m) + ":" + String(s) + "  Date: " +
                       day + "." + month + "." + year + " [" + dayOfWeek + "]"));

}


   /*   void getTime()
      {
       // WiFiClient client;
        DEBUG(Serial.print("connecting to www.google.com ...");)
        if(!client.connect("www.google.com", 80)) {     //"ntp.nic.cz"    "www.google.com"
          DEBUG(Serial.println("pripojeni spatne");)
          return;                                  }

        client.print(String("GET / HTTP/1.1\r\n") +
                    String("Host: www.google.com\r\n") +   //"Host: www.google.com\r\n"
                    String("Pripojeni: close\r\n\r\n"));

        int repeatCounter = 10;
        while (!client.available() && repeatCounter--) {
          delay(200); DEBUG(Serial.println("y."));     }

        String line;
        client.setNoDelay(false);
        int dateFound = 0;
        while(client.connected() && client.available() && !dateFound) {
          line = client.readStringUntil('\n');
          line.toUpperCase();
          // Date: Thu, 19 Nov 2015 20:25:40 GMT
          if(line.startsWith("DATE: ")) {
            localMillisAtUpdate = millis();
            dateFound = 1;
            date = line.substring(6, 22);
            date.toUpperCase();
            decodeDate(date);
            //Serial.println(line);
            h = line.substring(23, 25).toInt() + utcOffset; // + utcOffset; přidano GPT
            m = line.substring(26, 28).toInt();
            s = line.substring(29, 31).toInt();
//-------------------------GPT---------------------
      if (h >= 24) {
      h -= 24;
      day += 1;
      // můžeš přidat i přepočet měsíce a roku, pokud chceš být přesný
    }



//--------------GPT--------------------------

            DEBUG(Serial.println(String(h) + ":" + String(m) + ":" + String(s)+"   Date: "+day+"."+month+"."+year+" ["+dayOfWeek+"] "+(utcOffset)+"h");)
            localEpoc = h * 60 * 60 + m * 60 + s;
                                        }                           }
        client.stop();
     
     
      }
*/
// =======================================================================
// decodes: day, month(1..12), dayOfWeek(1-Mon,7-Sun), year
      void decodeDate(String date)
      {
        switch(date.charAt(0)) {
          case 'M': dayOfWeek=1; break;
          case 'T': dayOfWeek=(date.charAt(1)=='U')?2:4; break;
          case 'W': dayOfWeek=3; break;
          case 'F': dayOfWeek=5; break;
          case 'S': dayOfWeek=(date.charAt(1)=='A')?6:7; break;
                                }
        int midx = 6;
        if(isdigit(date.charAt(midx))) midx++;
        midx++;
        switch(date.charAt(midx)) {
          case 'F': month = 2; break;
          case 'M': month = (date.charAt(midx+2)=='R') ? 3 : 5; break;
          case 'A': month = (date.charAt(midx+1)=='P') ? 4 : 8; break;
          case 'J': month = (date.charAt(midx+1)=='A') ? 1 : ((date.charAt(midx+2)=='N') ? 6 : 7); break;
          case 'S': month = 9; break;
          case 'O': month = 10; break;
          case 'N': month = 11; break;
          case 'D': month = 12; break;
                                  }
        day = date.substring(5, midx-1).toInt();
        year = date.substring(midx+4, midx+9).toInt();
        return;
      }

// ================================ Letni čas =======================================
    //  no
 // ============ podrogramy ========================


    void updateTime()
     {
  long curEpoch = localEpoc + ((millis() - localMillisAtUpdate) / 1000);
 // long temp_value_2 = round(curEpoch  * (utcOffset) + 86400L); // long temp_value_2 = round(curEpoch +3600 * (utcOffset) + 86400L);
  long epoch = (long)(round(curEpoch  * utcOffset + 86400L)) % 86400L;  // long epoch = (long)(round(curEpoch + 3600 * utcOffset + 86400L)) % 86400L;
 // long epoch = temp_value_2 % 86400L;

  h = ((epoch  % 86400L) / 3600) % 24;
  m = (epoch % 3600) / 60;
  s = epoch % 60;
    
     if (h >= 24) {
      h -= 24;
      dayOfWeek += 1;}
      //day += 1;}
  //---------------------------------------------

   /*    
    Serial.println(day);
   // Serial.println(weekNames);
    Serial.println(dayOfWeek);
    //Serial.printf("%02d:%02d:%02d\n");
  
      Serial.println("UTC time from Google:");
      Serial.printf("%02d:%02d:%02d\n", h - utcOffset, m, s);
      Serial.println("Local time:");
      Serial.printf("%02d:%02d:%02d\n", h, m, s);
    */ 
//----------------------------------------------------------

    }  
// =======================================================================



      void showDigit(char ch, int col, const uint8_t *data)
      {
        if(dy<-8 | dy>8) return;
        int len = pgm_read_byte(data);
        int w = pgm_read_byte(data + 1 + ch * len);
        col += dx;
        for (int i = 0; i < w; i++)
          if(col+i>=0 && col+i<8*NUM_MAX) {
            byte v = pgm_read_byte(data + 1 + ch * len + 1 + i);
            if(!dy) scr[col + i] = v; else scr[col + i] |= dy>0 ? v>>dy : v<<-dy;
                                          }
      }
 
// =======================================================================

      void setCol(int col, byte v)
      {
        if(dy<-8 | dy>8) return;
        col += dx;
        if(col>=0 && col<8*NUM_MAX)
          if(!dy) scr[col] = v; else scr[col] |= dy>0 ? v>>dy : v<<-dy;
      }

// =======================================================================

      int showChar(char ch, const uint8_t *data)
      {
        int len = pgm_read_byte(data);
        int i,w = pgm_read_byte(data + 1 + ch * len);
        for (i = 0; i < w; i++)
          scr[NUM_MAX*8 + i] = pgm_read_byte(data + 1 + ch * len + 1 + i);
        scr[NUM_MAX*8 + i] = 0;
        return w;
      }

// =======================================================================

        void printCharWithShift(unsigned char c, int shiftDelay) {
          
          if (c < ' ' || c > '~'+25) return;
          c -= 32;
          int w = showChar(c, font3x7);
          for (int i=0; i<w+1; i++) {
            delay(shiftDelay);
            scrollLeft();
            refreshAll();          }                            }

// =======================================================================

      void printStringWithShift(const char* s, int shiftDelay){
        while (*s) {
          printCharWithShift(*s, shiftDelay);
          s++;     }                                          }
   //=============== konec ============================


    //***************vychod a zapad slunce ********
         // -----Propočet času HH:mm format
        char * hoursToString(double h, char *str)
        {
          int m = int(round(h * 60));
          int hr = (m / 60) % 24;
          int mn = m % 60;

          str[0] = (hr / 10) % 10 + '0';
          str[1] = (hr % 10) + '0';
          str[2] = ':';     //:
          str[3] = (mn / 10) % 10 + '0';
          str[4] = (mn % 10) + '0';
          str[5] = '\0';
          return str;
        } 

// ========================konec========================================
