
//  27.9.2024-10.8.2026
/*
    pouzite piny: 
      pinCidlaDS 5 =(senzoryDS)  Dalas 3x I2s= teplotni čidla
    // DATA_PIN 15   for  MD_Parola P = MD_Parola = nepouzité
      DIN_PIN 15 =display  (Led matrix 64x16)
      CS_PIN  13 =display  (Led matrix 64x16)
      CLK_PIN 12 =display  (Led matrix 64x16)
      analogRead(A0) =fotosenzor
      buzzer = D2; GPIO 4 // piezo
     // #define RF_PIN 2
*/
   // #include <NTPClient.h>
   // #include "note.h" 
   // #include <Timezone.h>
      #include "melode1.h"
      
    //  #include "note.h"
      #include <SolarCalculator.h>
      #include <ESP8266WiFi.h>
      #include <ESP8266HTTPClient.h>
    
      #include <WiFiUdp.h>
      #include "fonts.h"   
      #include <OneWire.h>
      #include <DallasTemperature.h> //<DS18B20Events.h>
      const int pinCidlaDS = 5;
      OneWire oneWireDS(pinCidlaDS);// vytvoření instance oneWireDS z knihovny OneWire
      DallasTemperature senzoryDS(&oneWireDS);// vytvoření instance senzoryDS z knihovny DallasTemperature

      #define TIMEDHT 3000   //4500 zpozdeni A0 fotoresistor
      #define NUM_MAX 16         
      #define LINE_WIDTH 64    
   
  // for D1 mini
      #define DIN_PIN 15  
  //  #define DATA_PIN 15    //for  MD_Parola P = MD_Parola
      #define CS_PIN  13  
      #define CLK_PIN 12  
 //------------------------senzory dalas-------------------
        #define MAX_SENSORS 3
        #define TEMP_MIN -120
        #define TEMP_MAX 120
        #define MAX_ERROR_COUNT 3
        #define CONVERSION_TIME 750   // 12bit převod 750 // 200 //188 ms 10bit čtení

            // sem vložte VAŠE skutečné adresy
        DeviceAddress adrVenku     = { 0x28, 0xE9, 0x90, 0xED, 0x74, 0x23, 0x0B, 0x07 };
        DeviceAddress adrBalkon    = { 0x28, 0xC3, 0xEC, 0x16, 0x75, 0x23, 0x0B, 0x7C };
        DeviceAddress adrRadiator  = { 0x28, 0x3F, 0x1F, 0xF2, 0x74, 0x23, 0x0B, 0x77 };

          float teploty[3] = {0.0, 0.0, 0.0};
          float posledniPlatna[3] = {0.0, 0.0, 0.0};
// ------------pouze při testu chyb dalas------------------------------
    //     unsigned long chybaPocet[3] = {0, 0, 0};
    //     unsigned long chybaCelkem = 0;

          bool konverzeBezi = false;
          unsigned long casZahajeni = 0;
          unsigned long posledniMereni = 0;
    //      unsigned long pocetMereni = 0;
      
  //*************************** *************************************   
      #define MAX_DEVICES 16
      #define ROTATE  90     
      #define DEBUG(x)    // nechat 
      #include "max7219.h"   //nehybat nepojede program ~~
     int color = 0;  // klopny obvod pro svetlo zap a vyp 
     int zrcadlo = 0;// klopny obvod pro zrcadlo zap 
     int ventilator2 = 0;// klopny obvod pro ventilator2 
     int pipej  = 0;// klopny obvod pro pipani v poledne
     int foto = 0;    //klopny obvod pro fotočidlo
    //  int pipejRF = 0;

    // =======================================================================
            const char* ssid     = "XXXXX";     // SSID of local network
            const char* password = "XXXXXXXXXXXX";   // Password 
    // =======================================================================
     long utcOffset = 2;   //  nejede nechat 1 = zima, 2= leto 
     long localEpoc = 0;   // 2 časový posun (utcOffset) 0 PŘIDÁNO NA 1 OPOŽDUJE SE DEN O 1 HOD
     long localMillisAtUpdate = 0; 
     
     unsigned long lastTempRequest = 0;
     const unsigned long tempInterval = 10000; // každých 10 sec
     
 //-------------------GPT  senzory dalas--------------------
  float nova;
    void zpracujCidlo(int index, float nova);
    DeviceAddress nalezeneAdresy[MAX_SENSORS];
        int nalezeno = 0;

        bool stejnaAdresa(const DeviceAddress a, const DeviceAddress b) {
          for (byte i = 0; i < 8; i++) {
            if (a[i] != b[i]) return false;
          }
          return true;
        }
//  --------------------------------------------------------
 //   WiFiUDP ntpUDP;
    HTTPClient http;
    WiFiClient client;
 //   NTPClient ntpClient(ntpUDP, "cz.pool.ntp.org", 0, 3600000);  //utcOffset * 3600// Aktualizace každou minutu(hodinu*60)
   
// ================ Hudební čas ===================================================
  // ====== Časy a počet ======
    const int playTimes[] = { 845, 1900, 1915 }; // 8:45, 19:15, 19:30
    const int playTimesCount = sizeof(playTimes) / sizeof(playTimes[0]);
  // const int melodiesAtTimes[] = { 2, 0, 2 }; // první čas = melodie 0, druhý = 1, třetí = 2

// ================ Hudební čas = konec   =========================================   

          // Location  Karvina
      double transit, sunrise, sunset;
      double latitude  = 49.8672914;
      double longitude = 18.5555467;

   
      uint8_t degC[] = { 6, 3, 3, 56, 68, 68, 68 }; // Deg C
      
        char str[10]; //oprava puvodně 0 
        char mereni1[6];  //oprava puvodne 2  // [2]fotorezistor
      
      byte h, m, s, day, month, dayOfWeek;  // upraveno GPT
       int year; 

        String date;
        String buf="";
        
        int clockOnly = 0;
        float teplota  = .01f;
        float teplota2 = .01f;
        float teplota3 = .01f;
        uint32_t timerDHT = TIMEDHT;
        int soucet;
        int mi;
        int prumer;
        #include "panacek.h"
        #include "vlacek.h"
     //------------------------------
        #define MAX_DIGITS 16
        byte dig[MAX_DIGITS]={0};
        byte digold[MAX_DIGITS]={0};
        byte digtrans[MAX_DIGITS]={0};
        long dotTime = 0;
        long clkTime = 0;
        int dx=0, dy=0;
        int xPos=0, yPos=0;
        byte del=0;


//*************** nastaveni  void setup() *******************
   void setup()  {
      WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
      buf.reserve(500);
      Serial.begin(115200);
      initMAX7219();
 
      sendCmdAll(CMD_SHUTDOWN, 1);      // zapni display
      sendCmdAll(CMD_INTENSITY, 0); 

      DEBUG(Serial.print("Connecting to WiFi ");)
      WiFi.begin(ssid, password);
      clr();
      xPos=2;
      yPos=0;
   
      printString("..WI-FI PRIPOJENI..", font3x7);
      refreshAll();
      while (WiFi.status() != WL_CONNECTED)  {
      delay(500); DEBUG(Serial.print("."));  }
                                                
      clr();
      yPos=1;
      xPos=10;
      DEBUG(Serial.println(""); Serial.print(" MyIP: "); Serial.println(WiFi.localIP());)
      printString((WiFi.localIP().toString()).c_str(), font3x7);
      refreshAll();
      
      senzoryDS.begin();
      
      senzoryDS.setResolution(12);       // 12bit
      senzoryDS.setWaitForConversion(false); //true blokující měření, bezpečnější
 
   //-----------počet senzoru----- -----------------------------  
      nalezeno = senzoryDS.getDeviceCount();
      Serial.print("Nalezeno senzoru: ");
      Serial.println(nalezeno);

for (int i = 0; i < nalezeno && i < MAX_SENSORS; i++) {
    if (senzoryDS.getAddress(nalezeneAdresy[i], i)); }

  // vypnuto serial.print     //  { Serial.print("Adresa ");
                              //  Serial.print(i);
                              //  Serial.print(": ");
                              //   for (byte j = 0; j < 8; j++) {
                              //    if (nalezeneAdresy[i][j] < 16) Serial.print("0");
                              // Serial.print(nalezeneAdresy[i][j], HEX); 
                              //   }
   // vypnuto serial.print    //     Serial.println();
      
      //--------------konec počet senzoru ---- ----------------

  //----Nastavení časového pásma ČR + automatický letní čas------
                configTime(0, 0, "pool.ntp.org");
                setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
                tzset();

                // POČKEJ na synchronizaci času
                Serial.print("Cekam na NTP cas");
                time_t now = time(nullptr);
                while (now < 100000) {   // dokud neni realny cas
                  delay(500);
                  Serial.print(".");
                  now = time(nullptr);
                }
                Serial.println(" OK");

      //  Serial.println( tm_isdst);
      //    Serial.println(tzset());

    } //-------nesahat------ ------------- 
// -----------Konec---void setup()----------------------------- 

            int dots,mode;  

// ================ Hlavni smyčka void loop() ==== Nasteveni a zapnutí 4x Display ===================
      void loop() {
          unsigned long curTime = millis();
          dots = (curTime % 1000) < 500;          // blikání tečky
          mode = (curTime % 60000) / 15000;       // 15 sec režim změny displeje
//-------------------------------------------------
 
   // ---------- Aktualizace času ----------
            
     //   checkAndPlayMelody(h, m); // podle tvého RTC nebo časového zdroje
        updateTime();
        checkAndPlayMelody();
        updateMelody();                       // neblokující přehrávání
                
  // --------- NON BLOCKING DS18B20 ------ Čtení čidel ----------


      if (!konverzeBezi && millis() - posledniMereni >= tempInterval)
      {
          posledniMereni = millis();
   
        // -----pouze test dalas měření------- 
        //  pocetMereni++;
        //  Serial.print("MERENI #");
        //  Serial.println(pocetMereni);
         
         
          senzoryDS.requestTemperatures();
          casZahajeni = millis();
          konverzeBezi = true;
      }
       
        else if (millis() - casZahajeni >= CONVERSION_TIME) {

            konverzeBezi = false;

            nova = senzoryDS.getTempC(adrVenku);
            zpracujCidlo(0, nova);

            nova = senzoryDS.getTempC(adrBalkon);
            zpracujCidlo(1, nova);

            nova = senzoryDS.getTempC(adrRadiator);
            zpracujCidlo(2, nova);

            teplota  = teploty[0];  // venku
            teplota2 = teploty[1];  // balkon
            teplota3 = teploty[2];  // radiator
        }
          
// ----konec----- NON BLOCKING DS18B20 ------ Čtení čidel ----------     

// ---------- Volba režimu displeje ----------
    // ===== PRIORITA TIMERU =====
     //   if (rfTriggered) {  drawRFCountdown(); } 
     //   else {
          switch(mode) {
                case 0: drawTime0(); break;
                case 1: drawTime2(); break;
                case 2: drawTime3(); break;
                case 3: drawTime1(); break;
            }       
       
    refreshAll();
     
    calcSunriseSunset(year, month, day, latitude, longitude, transit, sunrise, sunset);
   //----------------------------------------   
   }    //----nesahat------------------------
// ==============konec LOOP smyčky-----------------==============================================

   

    //  char* monthNames[] = {"LEDEN","UNOR","BREZEN","DUBEN","KVETEN","CERVEN","CERVENEC","SRPEN","ZARI","RIJEN","LISTOPAD","PROSINEC"};
        
        static const char weekNames[7][10] PROGMEM ={" PONDELI"," UTERY"," STREDA"," CTVRTEK"," PATECEK"," SOBOTKA"," NEDELE"};
        char txt[16];
        char txt1[16];
        char buffer[16];
        char buffer1[16];
       
    
//============== Display 0 Čidlo venku ===========================

         void drawTime0()
      {
           clr();
          yPos = 0;     //0 = nahoru
          xPos = 0;         
       
          sprintf(txt,"%02d",h); //GPT
          printString(txt, digits7x16);
          
          if(dots) printCharX(':', digits5x16rn, xPos);
          xPos +=2;
          //xPos+=(h>=22 || h==20)?1:2;
          sprintf(txt,"%02d",m);
          printString(txt, digits7x16);
        //--------------------------------------
        
//------------  Poledne --PIPEJ --------------------------------------------------
           
         if (pipej == 0 && h == 12 && m == 0) { tone(4, 440, 120); delay (200); tone(4, 440, 120);  pipej  = 1;} 
         if (pipej == 1 && h == 12 && m == 1) { pipej = 0;}
 
     
 
 
//-------------------teplota  venku  *******************************************************
            yPos = 0;               //    y0 = nahoru
           xPos = 33;            //x vice je doprava 
          if (teplota <= 65 && teplota > 58)  { printString("  REKORD ", font3x7); } 
            if (teplota <= 58 && teplota > 55)  { printString("  HORI", font3x7);  }
            if (teplota <= 55 && teplota > 50)  { printString("  PALI ", font3x7); } 
            if (teplota <= 50 && teplota > 45)  { printString("  SAUNA", font3x7); } 
            if (teplota <= 45 && teplota > 40)  { printString("  HYC  ", font3x7); } 
            if (teplota <= 40 && teplota > 35)  { printString(" MOC HIC", font3x7);}
            if (teplota <= 35 && teplota > 30)  { printString("BUDE HIC", font3x7);}
            if (teplota <= 30 && teplota > 25)  { printString("  TEPLO", font3x7);}
            if (teplota <= 25 && teplota > 20)  { printString(" TEPLEJI", font3x7);} 
            if (teplota <= 20 && teplota > 15)  { printString(" CHLADNO", font3x7);}
            if (teplota <= 15 && teplota > 10)  { printString("   ZIMA", font3x7); }     
            if (teplota <= 10 && teplota > 5)   { printString("   KOSA", font3x7);}
            if (teplota <= 5 && teplota >= 0)   { printString("  MRAZIK", font3x7);} 
            if (teplota < 0 && teplota > -5)    { printString("   LEDIK", font3x7);}
            if (teplota <= -5 && teplota >-10)  { printString("   NANUK", font3x7); }
            if (teplota <= -10 && teplota >-15) { printString("    RUMIK", font3x7); }
            if (teplota <= -15 && teplota >-20) { printString("    MRAZAK", font3x7);} 
            if (teplota <= -20 && teplota >-25) { printString("SARKOFAG", font3x7);} 
            if (teplota == -99)                { printString("   ERROR", font3x7); 
              sendCmd(12,10,0);sendCmd(13,10,0);sendCmd(14,10,0); sendCmd(15,10,0);}
                      
   //****************** Teplota venku ********************************************
           yPos = 1;    // y0 = nahoru   y1 = dolu //xPos x  vice je doprava 
               
              //if (teplota >=10 ) { xPos =41; } 
              if (teplota >=10 ) { xPos =39; } 
              if (teplota >= 0 && teplota <  10 ) { xPos =43; } //42
              if (teplota < 0 && teplota > -9) { xPos =40; }  //xPos =41 //38
              if (teplota < -9 && teplota > -10) { xPos =39; } 
              if (teplota <= -10 ) { xPos =38; }
              
          else  { sprintf(txt,"%.1f",teplota);printString(txt,font3x7 ); printString("'C", font3x7);}
                  
            if (teplota == -99)    { xPos = 43; printString("CIDLO", font3x7); }  //xPos = 42;   

     //  ZAJIMAVE NEJEDE TEPLOTA POKUD ODKOMENTUJI a hazi error
     // for(int i=64;i<96;i++) scr[96+i]<<=3;    //  <<=1    vyška řadku :-) <<=1 je dole  vice je dolu
     // for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;
yield(); // krmení watchdogu  ????
        } //nesahat
//--------------konec Display 0------------------------------------------

//============== Display 1 ====Balkon ======================
    void drawTime1()
    {
       clr();
        yPos = 0;
        xPos = 0;
      
        sprintf(txt,"%02d",h); // gpt
                printString(txt, digits5x16rn);
        if(dots) printCharX(':', digits5x16rn, xPos);
        xPos += 2;
        //xPos+=(h>=22 || h==20) ? 1 : 2;    //GPT   //x vice je doprava 
        sprintf(txt,"%02d",m);
        printString(txt, digits5x16rn);
        sprintf(txt,"%02d",s);
        printString(txt, font3x7);
//-------------Teplota2 Balkon ----------------------------
       yPos = 0;
       xPos = 38;  //   37            //x vice je doprava 
        printString("BALKON", font3x7);  

      yPos = 1;     // xPos = 43;    //41            //x vice je doprava    
     
    
      if (teplota2 >=10  ) { xPos =39; } 
      if (teplota2 >= 0 && teplota2 <  10 ) { xPos =42; } //42
      if (teplota2 < 0 && teplota2 >= -9) { xPos =40; }  //xPos =41 //38
      if (teplota2 <= -10 && teplota2 > -30) { xPos =38; }
   //    if (teplota2 == -99)    { xPos = 43;}
            
        sprintf(txt,"%.01f",teplota2); // (teplota2=balkon) (teplota) čidlo venkovní
        printString(txt, font3x7);    
        printString("'C", font3x7);   
      // for(int i=64;i<128;i++) scr[128+i]<<=3;    //  <<=1    vyška řadku :-) <<=1 je dole  vice je dolu
       //  for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;    //  <<=1    vyška řadku :-) <<=1 je dole  vice je dolu
    //============== Display 1 ====Balkon ====================== 

//*******************automaticke zrcadlo -------------- Display 1*************************************************************
    //--------------denne zrcadlo zap 10 hod ------------------------
      if ( zrcadlo == 0 && h == 10 && m == 0) { zrcadlo = 1;  // printString("zrcadlo:", font3x7); 
          http.begin(client,"http://192.168.0.200/5/on"); http.GET();  http.end();} 
            //  http.begin(client,"http://192.168.0.210/16/on"); http.GET();  http.end();} // zapni cool
      if ( zrcadlo == 1 && h == 10 && m == 1) {  zrcadlo = 0; } 

    // ----------sobota zrcadlo zap 7hod ---------------------
      if ((dayOfWeek-1) == 5 && h == 7 && m == 0) { zrcadlo = 1;  // sobota (weekNames-1)==5
          http.begin(client,"http://192.168.0.200/5/on"); http.GET();  http.end();} // zapni cool(zrcadlo) 
      if ( zrcadlo == 1 && h == 7 && m == 1) {  zrcadlo = 0; } 
    // ---------------nedele zrcadlo zap  7hod ---------------
      if ((dayOfWeek-1) == 6 && h == 7 && m == 0) { zrcadlo = 1;  // nedele  (weekNames-1)==6
          http.begin(client,"http://192.168.0.200/5/on"); http.GET();  http.end();} // zapni cool(zrcadlo) 
      if ( zrcadlo == 1 && h == 7 && m == 1) {  zrcadlo = 0; }
     // Serial.println (dayOfWeek-1); //sobota ==5
  yield(); // krmení watchdogu

      }//---------------nesahat-----------------------------
//---------------Konec DISPAY 1-----------------------------

//============== Display 2 == Zapad východ ========================
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
      // Wait for a time TIMEDHT = 4500
          if ((millis() - timerDHT) > TIMEDHT) {
            // Update the timer
            timerDHT = millis(); // puvodne konec smyčky  
            int mereni = analogRead(A0); //cteni hodnoty fotorezistoru
            prumer = mereni /15 ;            } // prumer = (mereni / 15);

    //--------konec smyčky foto odporu  (mereni) 

//--------auto nastaveni jasu podle deniho svetla ----fotoodpor =68 - jas display =4----   
    
      if(prumer < 10 )  {foto = 0;}
      if(prumer > 30 && prumer < 40 )  {foto = 2;}
      if(prumer > 40 && prumer < 50 )  {foto = 3;}
      if(prumer > 50 && prumer < 60 )  {foto = 8;}
      if(prumer > 60 && prumer < 70 )  {foto = 10;}
 sendCmd(12,10,foto);sendCmd(13,10,foto);sendCmd(14,10,foto);sendCmd(15,10,foto); 
  //   sendCmd(11,10,foto);  //(pozice,jas,intesita jasu) // intesita jasu = nastaveni 0= min ,15 max 
  // Serial.println( prumer);
 
  //-----konec ---nastaveni jasu podle deniho svetla --------------- 


  //---------------------zapnout a vypnout svetla  podle foto odporu  *********************************--------

    if (color == 1 && prumer > 26 && prumer < 30 && h >=4  && h <=8) {  color = 0;// printString("VYCHOD:", font3x7); 
          http.begin(client,"http://192.168.0.210/4/off"); http.GET();  http.end(); // vypni color
          http.begin(client,"http://192.168.0.210/13/off"); http.GET();  http.end(); }// vypni color
 //       sendCmd(12,10,foto);sendCmd(13,10,foto);sendCmd(14,10,foto); sendCmd(15,10,foto);      
    if (color == 0 && prumer < 30 && prumer > 26 && h >=15 && h <=22) {  color = 1; //printString("ZAPAD:", font3x7); 
          http.begin(client,"http://192.168.0.210/4/on"); http.GET();  http.end(); // zapni color
          http.begin(client,"http://192.168.0.210/13/on"); http.GET();  http.end(); }// zapni color
   //    sendCmd(12,10,0);sendCmd(13,10,0);sendCmd(14,10,0); sendCmd(15,10,0);   
      xPos = 53;  //54
      yPos = 0;  
      if (color == 0) {printString(")", font3x7); sprintf(mereni1,"%d",prumer); printString(mereni1,font3x7); }// ")" změněno na "|" s mezerama
      if (color == 1) {printString("/", font3x7); sprintf(mereni1,"%d",prumer); printString(mereni1,font3x7); }// "/" zmeněno na "|"
      
      for(int i=0;i<32;i++) scr[32+i]<<=1;    //  <<=1    vyška řadku :-) <<=1 je dole  vice je dolu
      // for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;    //  <<=1    vyška řadku :-) <<=1 je dole  vice je dolu   
                                          
   

    //-------------------------- slunce východ a zapad --------------
    
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
       sprintf(buffer,"%02d.%02d.%2d",day,month,year-2000);
       printString(buffer,font3x7); 
           //  for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;    //  <<=1    vyška řadku :-) <<=1 je dole  vice je dolu
          
//--------------automat  ++ DEN V TYDNU V LEVO DOLE ---------------------------------------
       yPos = 1;    //y1 =  dolu
       xPos = 1;
     bool zobrazeno = false;
     bool vlacekBylVCase = false;
     
      if        (month==12 && day==31 && h==12 && m<=10) { printString("SILVESTR", font3x7); zobrazeno = true;}
      else if   (month==12 && day==31 && h==23 && m>=55) { printString("SILVESTR", font3x7); zobrazeno = true;}
      else if   (month==1  && day==1  && h==0)  { printString("NOVY ROK", font3x7); zobrazeno = true;}
      else if   (h==12 && m<3)  { printString(".POLEDNE.", font3x7); zobrazeno = true;}
      else if   (month==8 && day==27 && h==12 && m>=0 && m<=30) { printString("MILACEK", font3x7); zobrazeno = true;}
      else if   (h==19 && m>=0 && m<=15) { printString(" PRASKY", font3x7); zobrazeno = true;} 
      else if   (h==8 &&  m>=45 && m<=59){ printString("VEM LEKI", font3x7); zobrazeno = true;} 
      else if   (jeCasPacman()) {    drawPacman();    zobrazeno = true; }
    //   else if (h >= 17 && m <= 59){   if (!panacekBylVCase) 
     //    {   startPanacek();  panacekBylVCase = true;  }
     //       drawPanacek(); zobrazeno = true; }  else  {      panacekBylVCase = false;  }
    
     //   else if (h >= 18 && m <= 59) { if (!vlacekBylVCase) 
     //           { startVlacek(); vlacekBylVCase = true; } 
      //          drawVlacek(); zobrazeno = true; } else { vlacekBylVCase = false; }
    
     // else if (h >= 17 && m <= 59){
     //      if (!panacekAktivni)   startPanacek();  drawPanacek();  zobrazeno = true;}
   
   // else if (h >= 17 && m <= 59) {  drawPanacek();  zobrazeno = true;
    
    //  else if    (h>=17 && m >=0 && m <=59 ) {startPanacek();drawPanacek();zobrazeno = true;}
   //   if        (!jeCasPacman())   { pacmanAktivni = false;   }

   // else if   (h==18 &&  m>=10 && m<=11)  { printString("TESTIK", font3x7); zobrazeno = true;} 
           // strcpy_P(txt, weekNames[dayOfWeek-1]);printString(txt, font3x7);
  
          
         if (!zobrazeno) {
         yPos = 1;
         xPos = 1;
         sprintf(txt, "%s", weekNames[dayOfWeek-1]); printString(txt, font3x7); }
       
       for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1; 
 
  yield(); // krmení watchdogu
     }      // nesahat------------
//***************Konec DISPAY 2*********************************************  

//---------display 3 *************----drawTime3------Radiator -========================================================
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

         // 

  //***********automat  --zima --- teplota radiator ventilator >39 stupnu Celsia**************************
               //  Serial.println(ventilator2);
                // Serial.println(teplota3); //radiator  // teplota = venku
       if (month < 6 && month >9) { // říjen,listopad,prosinec,leden,unor,březen,duben,kveten
            yPos = 1;               //    y0 = nahoru
            xPos = 0;            //x vice je doprava 
            if (teplota3 > 45)                   { printString("TOPI HURA", font3x7); } 
            if (teplota3 <= 45 && teplota3 > 40) { printString("TOPI MINI", font3x7); } 
            if (teplota3 <= 40 && teplota3 > 35) { printString(" TEPLO ", font3x7);printString("!",digits5x8rn); }  // srdce
            if (teplota3 <= 35 && teplota3 > 30) { printString(" VICE ", font3x7);printString("(#)",digits5x8rn);}    //$ nahoru 
            if (teplota3 <= 30 && teplota3 > 25) { printString("MALO ", font3x7);printString("($)",digits5x8rn);}    // rovne
            if (teplota3 <= 25 && teplota3 > 20) { printString("NETOPI", font3x7); printString("(%)",digits5x8rn);}    // dolu
            if (teplota3 <= 20 && teplota3 > 15) { printString(" CHLADNO", font3x7); }
            if (teplota3 <= 15 && teplota3 > 10) { printString("    ZIMA", font3x7); }       
            if (teplota3 <= 10 && teplota3 > 0 ) { printString("ZMRZNEM ", font3x7); }  
            if (teplota3 <= -99)                 { printString("ERR.CIDLO ", font3x7); }  
           
 
            if  (ventilator2 == 0 && h >= 8 && teplota3 >= 39 ) { ventilator2 = 1;
              http.begin(client,"http://192.168.0.200/2/on"); http.GET();  http.end(); }// zapni ventilator2
            if  (ventilator2 == 1 && teplota3 < 38 ) { ventilator2 = 0;
              http.begin(client,"http://192.168.0.200/2/off"); http.GET();  http.end(); }// vypni ventilator2
            if (ventilator2 == 1 && h>=21) { ventilator2 = 0;
              http.begin(client,"http://192.168.0.200/2/off"); http.GET();  http.end();}// vypni ventilator2
    } 
  //***automat konec --zima-- teplota radiatoru ventilator >39 stupnu Celsia**************************   
  
    
 //***********automat  --leto --- teplota venku ventilator >39 stupnu Celsia**************************

     if (month > 4 && month <10) {    // květen,červen,červenec,srpen,září
            yPos = 1;               //    y0 = nahoru
            xPos = 0;               //    x vice je doprava 
          if (month == 5)  { printString(" KVETEN", font3x7);}
          if (month == 6)  { printString(" CERVEN", font3x7);} //   xPos = 42;    printString("!",digits5x8rn);   // srdce 
          if (month == 7)  { printString("CERVENEC", font3x7);} 
          if (month == 8)  { printString("  SRPEN", font3x7);} 
          if (month == 9)  { printString("  ZARI  ", font3x7);printString("!",digits5x8rn);} // srdce

          if  (ventilator2 == 0 && h >= 8 && teplota >= 39 ) { ventilator2 = 1;
              http.begin(client,"http://192.168.0.200/2/on"); http.GET();  http.end(); }// zapni ventilator2
          if  (ventilator2 == 1 && teplota < 38 ) { ventilator2 = 0;
              http.begin(client,"http://192.168.0.200/2/off"); http.GET();  http.end(); }// vypni ventilator2
          if (ventilator2 == 1 && h>=21) { ventilator2 = 0;
              http.begin(client,"http://192.168.0.200/2/off"); http.GET();  http.end(); }// vypni ventilator2
      }

          yPos = 0;   //    y0 = nahoru   y1 = dolu 
          xPos = 34;   //34  x vice je doprava
          printString("RADIATOR", font3x7);
          yPos = 1;     // =0
          xPos = 39;   // 42             
          sprintf(txt,"%.1f",teplota3);
          printString(txt,font3x7 ); 
          printString("'C", font3x7); 
         
           for(int i=0;i<LINE_WIDTH;i++) scr[LINE_WIDTH+i]<<=1;

  //******konec automat -leto--- teplota(0) venku ventilator >39 stupnu Celsia **************   
          
//----------------------konec display 3----------------------------------------------
     }    // nesahat
//*************--------------konec display 3 ----------------------  
// ============ podrogramy ============================================================================================ 

   // ====== Kontrola a spuštění správné melodie a času ======
       void checkAndPlayMelody() {
         static int lastPlayedMinute = -1;
         int key = h * 100 + m;
         for (int i = 0; i < playTimesCount; i++) {
         if (key == playTimes[i] && m != lastPlayedMinute) {

       //    startMelody(melodiesAtTimes[i]);
         startMelody();
         lastPlayedMinute = m;
         break;  } 
              }
    // reset při změně minuty
          if (m != lastPlayedMinute && player.playing == false) {
          lastPlayedMinute = -1; }
               }
  
   // ====konec== Funkce pro kontrolu a spuštění melodie ====== 

//*************************čteni fontu *********************************************

   
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
      { int wd=0;
        while(*s) wd += 1+charWidth(*s++, font);
        return wd-1;}
      

    // =======================================================================

      int stringWidth(String str, const uint8_t *font)
      { return stringWidth(str.c_str(), font);}
      

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
           if(x+i<LINE_WIDTH) scr[x+LINE_WIDTH*(j+yPos)+i]=0;}
         return w;}
      

      // =======================================================================

      void printChar(unsigned char c, const uint8_t *font){
        
      if(xPos>NUM_MAX*8) return; int w = printCharX(c, font, xPos); xPos+=w+1;}
        
      // =======================================================================

          void printString(const char *s, const uint8_t *font)
          {  while(*s) printChar(*s++, font); }
           
         void printString(String str, const uint8_t *font)
          { printString(str.c_str(), font);  }

 // ===================== Den v Tydnu ==================================================
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
    
//----------------updateTime------------------------------------
         void updateTime()
        {
          time_t now = time(nullptr);
          struct tm* timeinfo = localtime(&now);

          if (timeinfo == nullptr) return;

          h = timeinfo->tm_hour;
          m = timeinfo->tm_min;
          s = timeinfo->tm_sec;
          day = timeinfo->tm_mday;
          month = timeinfo->tm_mon + 1;
          year = timeinfo->tm_year + 1900;
          dayOfWeek = (timeinfo->tm_wday == 0) ? 7 : timeinfo->tm_wday;
        }  // nesahat  ----konec updateTime 

// =====================podprogramy=========================================
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
            refreshAll();  } 
                    }

      // =======================================================================
      void printStringWithShift(const char* s, int shiftDelay){
        while (*s) {
          printCharWithShift(*s, shiftDelay);
          s++;     }                                          }
     //===================================================================
    //******vychod a zapad slunce ----Propočet času HH:mm format
         
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


    //-----------------------------------------------------------------
   //------------------ čidlo dalas --------------------
    void zpracujCidlo(int index, float nova)
    {
        bool chyba =
            nova == DEVICE_DISCONNECTED_C ||
            nova == -127.0 ||
            nova == 85.0 ||
            nova < TEMP_MIN ||
            nova > TEMP_MAX;

        if (chyba) { return;  }

        teploty[index] = nova;
        posledniPlatna[index] = nova;
    }
//-------------konec----- čidlo dalas -------------------- 
  /* //------------------ čidlo  dalas ----serial test----------------
    void zpracujCidlo(int index, float nova)
  {
    bool chyba =
        nova == DEVICE_DISCONNECTED_C ||
        nova == -127.0 ||
        nova == 85.0 ||
        nova < TEMP_MIN ||
        nova > TEMP_MAX;

    if (chyba) {

        chybaPocet[index]++;
        chybaCelkem++;

        Serial.print("CHYBA cidlo ");

        if (index == 0) Serial.print("VENKU");
        if (index == 1) Serial.print("BALKON");
        if (index == 2) Serial.print("RADIATOR");

        Serial.print(" | hodnota = ");
        Serial.print(nova);

        Serial.print(" | chyby cidla = ");
        Serial.print(chybaPocet[index]);

        Serial.print(" | CELKEM = ");
        Serial.println(chybaCelkem);

        return;
    }

    teploty[index] = nova;
    posledniPlatna[index] = nova;

    Serial.print("OK ");
    if (index == 0) Serial.print("VENKU");
    if (index == 1) Serial.print("BALKON");
    if (index == 2) Serial.print("RADIATOR");

    Serial.print(" = ");
    Serial.print(nova, 1);
    Serial.print(" C | chyby = ");
    Serial.print(chybaPocet[index]);
    Serial.print(" | celkem = ");
    Serial.println(chybaCelkem);
    }
*/
// ========================konec========================================
