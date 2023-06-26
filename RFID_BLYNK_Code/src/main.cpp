#define BLYNK_TEMPLATE_ID "TMPL93daZvvU"
#define BLYNK_DEVICE_NAME "Test communication"
#define BLYNK_AUTH_TOKEN "oQQox5d3KSxWMidk2-RfrZcTDmuBuHiw"
#define BLYNK_PRINT Serial

#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>
#include <Servo.h>
Servo myservo;

//Konfigurowanie pinów do czytnika MRC522
#define RST_PIN         5          
#define SS_PIN          7      

char auth[] = BLYNK_AUTH_TOKEN;

#define W5100_CS  10
#define SDCARD_CS 4

//Definiowanie pinów do obsługi silnika krokowego
#define P1 6
#define P2 4
#define P3 3
#define P4 8

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Tworzenie instancji MFRC522
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

signed long aktualnyCzas = 0;
signed long zapamientanyCzas = 0;

//Inicjalizacja zmiennych do sterowanie servem oraz śilnikime korokowym 
int stan_serva=0;
int Stan_silnika=0;
int Obroty=0;
int P_silnika=0;
String RFID_data= "";

//*****************************************************************************************//

void setup() {
  Serial.begin(9600);                    // Inicjalizacja komunikacji szeregowej z komputerem PC
  SPI.begin();                           // Inicjalizacja magistrali SPI
  mfrc522.PCD_Init();                    // Inicjowanie karty MFRC522
  Serial.println(F("RFID Sorting:"));    // Pokazanie nazwy porojektu informujące o gotowości programu


  // Debug console
  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, HIGH); // Odznacz kartę SD
  Blynk.begin(auth, "blynk.cloud", 80);
  Blynk.virtualWrite(V0, 0);
  
  //Kofigujracja Serva
  myservo.attach(2);
  myservo.write(90);
 
  //Konfiguracja Silnika Krokowego
  pinMode (P1, OUTPUT);
  pinMode (P2, OUTPUT);
  pinMode (P3, OUTPUT);
  pinMode (P4, OUTPUT);

  Stan_silnika=0;
  Obroty=0;
  P_silnika=0;
}
 
//*****************************************************************************************//

void RFID()  // Podprogram odczytujący wybrane dane z pamięci karty RFID
{
  Serial.println(F("**Wykryro karte:**"));
        byte buffer1[18];
        byte block = 4;
        byte len = 18;

        status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK) 
        {
          Serial.print(F("Uwierzytelnianie nie powiodło się: "));
          Blynk.logEvent("bd_autoryzacji_rfid");
          Serial.println(mfrc522.GetStatusCodeName(status));
          return;
        }
      
        status = mfrc522.MIFARE_Read(block, buffer1, &len);
        if (status != MFRC522::STATUS_OK) 
        {
          Serial.print(F("Nieudany Odczyt: "));
          Blynk.logEvent("nieudany_odczyt_");
          Serial.println(mfrc522.GetStatusCodeName(status));
          return;
        }
      
        String value = "";
        for (uint8_t i = 0; i < 16; i++)
        {
            value += (char)buffer1[i];
            
        }
        Serial.print(value);
        RFID_data=value;
        Blynk.virtualWrite(V0, value);
        value.trim();
        
      
        Serial.println(F("\n**Zakończenie czytania danych**\n"));

      
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
}

void (* resetFunc) (void) =0; 

BLYNK_WRITE(V1) // Fukcja odpowiedzialna za reset arduino za pomocą Bylnka 
{
if (param.asInt()==1) 
{
  Blynk.logEvent("reset","Reset urządzenia");
  resetFunc();
  delay(100);
}
}

// void (* Events) (void) =0; 

// BLYNK_WRITE(V2) // Funkcja testowa
// {
//   if (param.asInt()==1) {
//   Blynk.logEvent("bd_autoryzacji_rfid");
//   Blynk.virtualWrite(V0, Stan_silnika);
//   delay(100);
//   Blynk.virtualWrite(V0, Obroty);
//   delay(100);
//   Blynk.virtualWrite(V0, P_silnika);
// }
// }

void (* Servo) (void); 

BLYNK_WRITE(V3) //Sterownie Serwem
{
int kat = param.asInt();
  if ((stan_serva - kat)<180) 
{
  myservo.write(kat);
  stan_serva=kat;
}
}

void Odliczanie ()
{
  if(aktualnyCzas - zapamientanyCzas >= 10000)
  {
    RFID_data = "0";
    zapamientanyCzas=aktualnyCzas; 
  }
}

void Servoo () //Sterownie Serwem
{

int stopnie;
  if (RFID_data=="1234567891234567") 
  {
    stopnie=45;
    myservo.write(stopnie);
    Blynk.virtualWrite(V3, stopnie);
    Odliczanie();
    
  }
  else if (RFID_data=="1234567891234568")
  {
    stopnie=135;
    myservo.write(stopnie);
    Blynk.virtualWrite(V3, stopnie);
    Odliczanie();
    
  }
  else 
  {
    stopnie=90;
    myservo.write(stopnie);
  }
 
}

void motor_P(int dly)  // Podprogram do sterowania prawymi obrotami silnika
{
  digitalWrite (P1, HIGH);
  delay(dly);
  digitalWrite (P1, LOW);
  digitalWrite (P2, HIGH);
  delay(dly);
  digitalWrite (P2, LOW);
  digitalWrite (P3, HIGH);
  delay(dly);
  digitalWrite (P3, LOW);
  digitalWrite (P4, HIGH);
  delay(dly);
  digitalWrite (P4, LOW);
}

void motor_L(int dly)  // Podprogram do sterowania lewymi obrotami silnika
{
  digitalWrite (P4, HIGH);
  delay(dly);
  digitalWrite (P4, LOW);
  digitalWrite (P3, HIGH);
  delay(dly);
  digitalWrite (P3, LOW);
  digitalWrite (P2, HIGH);
  delay(dly);
  digitalWrite (P2, LOW);
  digitalWrite (P1, HIGH);
  delay(dly);
  digitalWrite (P1, LOW);
}

//odczyt zadanego stanu silnika (czy jest właczony czy nie)
BLYNK_WRITE(V4) 
{
  Stan_silnika = param.asInt();
}

// odczyt zadanych obratów silnika (prawe czy lewe)
BLYNK_WRITE(V5) 
{
  Obroty = param.asInt();
}

//odczyt zadanej prędkości silnika 
BLYNK_WRITE(V6) 
{
  P_silnika = param.asInt();
}
  
//*****************************************************************************************//

void loop() {
 Blynk.run();
  //Przygotuj klucz - wszystkie klucze są ustawione na FFFFFFFFFFh przy dostawie chipa z fabryki.
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
  // Załącznie fukcji do odczytu dabych zawartych na karcie RFID
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()){

        RFID();

    }
    else
    {
      Serial.println("Blad karty");
      Blynk.logEvent("bd_karty");
    }
  }
  aktualnyCzas = millis();
  Servoo();
  // Sterowanie Silnikiem krokowym odpowiedzialnym za taśmociąg.
  if(Stan_silnika==0)
    { 
      Stan_silnika=0;
    }
    else{
      
      if(Obroty==0)
      {
        motor_P(P_silnika);
      }
      else
      {
        motor_L(P_silnika);
      }
    }
}