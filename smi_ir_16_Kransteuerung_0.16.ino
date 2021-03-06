//#include <Scheduler.h>

/*
 * Kransteuerung mit IRremote
 * Prototyp
 * Version 0.1 März 2017
 * 
 * voll funktionsfähig
 */

#include <IRremote.h>


const byte interruptPin=2;
int TIP120pin=3; // Set pin for output to control TIP120 Base pin
int RECV_PIN = 11;
long irWert=0;
long irStatus=0;
int keineTaste=0;
int taste=0;
boolean motorTasteGedrueckt=false;   // Variable wird true, sobald eine der Tasten:
                                 // Haken-hoch-runter / Laufkastze-vor-zurück / Kran drehen
                                 // gedrückt wurde
boolean programmsteuerung=false;       // Kranaktion wurde nicht durch Motor-Tasten ausgelöst


int outPin22=22;      // blaue LED => leuchtet, wenn das Progranmm hochgeladen und gestartet wurde
int outPin30=24;      // Haken hoch - grünes Krankabel
int outPin32=25;      // Haken runter - lila Krankabel
int outPin34=26;      // Laufkatze nach vorn - braun
int outPin36=27;      // Laufkatze nach hinten - gelb
int outPin38=28;      // Drehung links - weiss
int outPin40=29;      // Drehung rechts - blau


class kranMotor{

public:  String motorName;
private:  int motorOutPin1;
  int motorOutPin2;
  int motorZustand;    // 0=Aus / 1=vor / -1=rück 
  private:  long laufzeit=0;    // in Millisekunden 
  private:  long time1=0;
  private:  long delta_t=0;
  private:  boolean motorGesperrt=false;
  private:  long stromAusT1;
  private:  long stromAusDauer=5000;
  
  
  public: kranMotor(int MOP1, int MOP2, String _motorName)                             // Konstruktor mit 3 Parametern
                      : motorOutPin1(MOP1), motorOutPin2(MOP2), motorName(_motorName){
    Serial.print("kranMotor-Konstruktor: ");
    Serial.print(motorName);      
    Serial.print("             millis() = ");
    Serial.print(millis());      
    Serial.print("             Laufzeit = ");
    Serial.println(laufzeit);      
                        
                        };
  public: String getMotorName(){
      return motorName;
  };
  
  public: long getLaufzeit(){
      return laufzeit;
  }; 

   public: void vor(){

    String temp;
    if((motorGesperrt == true) and (millis()>(stromAusT1+stromAusDauer))){motorGesperrt=false;}
    if((motorGesperrt == true) and (millis()<(stromAusT1+stromAusDauer))){Serial.println("Motor gesperrt.");}
    if(motorZustand != 1 and motorGesperrt==false){
      digitalWrite(motorOutPin2, HIGH);
      digitalWrite(motorOutPin1, LOW);     
      
      if (motorZustand == 0){
        time1= millis();
      }
      temp = "Kranmotor-" + motorName + " Vor() time1 = " + String(time1) + "    millis()= " + String(millis());
      Serial.println(temp);  
      motorZustand=1;
    }
  }                                // void vor()

  public: void rueck(){
     
    String temp;
    if((motorGesperrt == true) and (millis()>(stromAusT1+stromAusDauer))){motorGesperrt=false;}
    if((motorGesperrt == true) and (millis()<(stromAusT1+stromAusDauer))){Serial.println("Motor gesperrt.");}
    if(motorZustand != -1 and motorGesperrt==false){
      digitalWrite(motorOutPin1, HIGH);
      digitalWrite(motorOutPin2, LOW);  
    
       if (motorZustand == 0){
         time1= millis();
       }
       temp = "Kranmotor-" + motorName + " Rueck() time1 = " + String(time1) + "    millis()= " + String(millis());
       Serial.println(temp);  
       motorZustand=-1;
    }
  }                              // void rueck()

  public: void aus(){
    // String temp;
    if (motorZustand != 0){ 
      digitalWrite(motorOutPin2, HIGH);
      digitalWrite(motorOutPin1, HIGH);
      motorZustand = 0;  
      laufzeit = laufzeit + millis()-time1;
    }
    Serial.print("Motor-Aus: ");
    Serial.print(motorName);      
    Serial.print("             millis() = ");
    Serial.print(millis());      
    Serial.print("             Laufzeit = ");
    Serial.println(laufzeit);      
  }                      // void aus()

  public: void stromAbschaltung(){
    this->aus();
    stromAusT1=millis();
    motorGesperrt=true;
  }

  
};   //class kranMotor


IRrecv irrecv(RECV_PIN);
decode_results results;
kranMotor* m2;
kranMotor* m3;
kranMotor* m1;


void setup()
{

  Serial.begin(19200);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(TIP120pin, OUTPUT); // Set pin for output to control
  pinMode(outPin22, OUTPUT);
  pinMode(outPin30, OUTPUT);
  pinMode(outPin32, OUTPUT);
  pinMode(outPin34, OUTPUT);
  pinMode(outPin36, OUTPUT);
  pinMode(outPin38, OUTPUT);
  pinMode(outPin40, OUTPUT);
  digitalWrite(outPin22, HIGH);
  digitalWrite(outPin30, HIGH);
  digitalWrite(outPin32, HIGH);
  digitalWrite(outPin34, HIGH);
  digitalWrite(outPin36, HIGH);
  digitalWrite(outPin38, HIGH);
  digitalWrite(outPin40, HIGH);

  attachInterrupt(digitalPinToInterrupt(interruptPin), ISR_0, HIGH);  /* Die Funktion digitalPinToInterrupt
                                                                         mappt den gewählten Interruptpin auf
                                                                         die richtige Interrupt_Nummer:

                                                                         URLs zum Nachlesen: 
                                                                         http://playground.arduino.cc/Deutsch/HalloWeltMitInterruptUndTimerlibrary
                                                                         https://www.tutorialspoint.com/arduino/arduino_interrupts.htm  
                                                                      */ 
 
  Serial.println("ZfsL-Kran Version 0.23");
  
  m1 = new kranMotor(outPin30, outPin32, "m1"); 
  m2 = new kranMotor(outPin34, outPin36, "m2");  
  m3 = new kranMotor(outPin38, outPin40, "m3"); 
 
}



void irSteuerung(){
    if (irrecv.decode(&results)) {
//  Serial.println(results.value, DEC);                             
    irWert = results.value, DEC;
//  Serial.println("______ IR-Wert: ");
//  Serial.println(String(irWert));
    irrecv.resume(); // Receive the next value
    keineTaste=0;
    }
  else{
    keineTaste = keineTaste +1;
//    Serial.println("keineTaste: ");
//    Serial.println(keineTaste);
    }
    delay(1);

  if (irWert == -1){ // or irWert == 3031753245) {
//  Serial.println("IF-1 - Wiederholung--------------------------: ");
    irWert=irStatus;
    keineTaste =0;
    }

 if ((keineTaste > 2) && (motorTasteGedrueckt)  ){
    motorTasteGedrueckt=false;
    allesAus();
  }


 if (irWert == (16738455)){ // or irWert == 3031753245) {
//  Serial.println("IF-0 - Null--------------------------: ");
    digitalWrite(outPin30, HIGH);
    digitalWrite(outPin32, HIGH);
    }

 if (irWert == 16754775){ // or irWert == 3031753245) {               // IR-Taste +   => Haken hoch
    irStatus = irWert;
    if (!motorTasteGedrueckt){
      m1->vor();
      motorTasteGedrueckt=true;}
    }

 if (irWert == (16769055)){ // or irWert == 3031753245) {             // IR-Taste: Minus => Haken runter
    irStatus=irWert;
    if (!motorTasteGedrueckt){
      m1->rueck();
      motorTasteGedrueckt=true;
    }
  }    

 if (irWert == (16712445)){ // or irWert == 3031753245) {               // IR-Taste: >>   => Laufkatze nach vorn
    irStatus=irWert;
    if (!motorTasteGedrueckt){
      m2->vor();
      motorTasteGedrueckt=true;
    }
  }
  if (irWert == 16720605){ // or irWert == 3031753245) {                // IR-Taste: <<  => Laufkatze zum Turm
     irStatus = irWert;
     if (!motorTasteGedrueckt){
      m2->rueck();
      motorTasteGedrueckt=true;
     }  
   }
  if (irWert == (16769565)){ // or irWert == 3031753245) {
    irStatus=irWert;
    if (!motorTasteGedrueckt){
      m3->vor();
      motorTasteGedrueckt=true;
    }   
  }
    
  if (irWert == (16753245)){ // or irWert == 3031753245) {
    irStatus = irWert;
    if (!motorTasteGedrueckt){
      m3->rueck();
      motorTasteGedrueckt=true;
    }
  }
}                       //void irSteuerung()

void allesAus(){

  m1->aus();
  m2->aus();
  m3->aus();
  
  irStatus=0;
  irWert=0;
  keineTaste=0;
  motorTasteGedrueckt=false;
}

void ISR_0(){                                    //Interrrupt Service Routine für IR0 auf PIN2 (PWM)
  m1->stromAbschaltung();
  Serial.println("Motor 1 - Abschaltung wegen zu hohem Motorstrom -");
  delay(6000);
}

void loop(){
 irSteuerung();
 delay(65);
}                                //loop
