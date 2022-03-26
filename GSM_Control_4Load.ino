#include <EEPROM.h>
#include <SoftwareSerial.h> //Create software serial object to communicate with SIM800L
SoftwareSerial GSM(8, 9);   //SIM800L Tx & Rx is connected to Arduino #8 & #9 // comunicacion del arduino al modulo sim800l por los pines 8 y 9 (tx, rx)

String phone_no1 = "+923378655465"; //change +92 with country code and 3378655465 with phone number to sms // variables para almacenar los numeros permitidos para recibir los mensajes
String phone_no2 = "Enter Number2"; 

String        RxString  = ""; // Will hold the incoming String  from the GSM shield // almacena el mesaje recibido por el modulo sim800
char          RxChar    = ' ';
int           Counter   = 0;
String        GSM_Nr    = "";
String        GSM_Msg   = "";

#define Relay1 2 // Load1 Pin Out // definicion de las variables de salida *modificar esto de acuerdo a la cantidad de salidas a utilizar como relé
#define Relay2 3 // Load2 Pin Out
#define Relay3 4 // Load3 Pin Out
#define Relay4 5 // Load4 Pin Out

int load1, load2, load3, load4;   // variable que almacena el estado de los pines de salida para almecenarlos en la eeprom

void setup(){ // put your setup code here, to run once

pinMode(Relay1, OUTPUT); digitalWrite(Relay1, 1);  // inicializando los pines de salida 
pinMode(Relay2, OUTPUT); digitalWrite(Relay2, 1); 
pinMode(Relay3, OUTPUT); digitalWrite(Relay3, 1); 
pinMode(Relay4, OUTPUT); digitalWrite(Relay4, 1); 

Serial.begin(9600);//Begin serial communication with Arduino and Arduino IDE (Serial Monitor)  //inicializar comunicacion con el IDE de arduino "mensajes de consola"
GSM.begin(9600);   //Begin serial communication with Arduino and SIM800L // inicializar la comunicacion el arduino y el modulo sim 

Serial.println("Initializing....");
initModule("AT","OK",1000);                //Scan for GSM Module // buscando la disponibilidad del modulo 
initModule("AT+CPIN?","READY",1000);       //this command is used to check whether SIM card is inserted in GSM Module or not // validando si el modulo tiene la simcard insertada
initModule("AT+CMGF=1","OK",1000);         //Set SMS mode to ASCII // estableciendo formato del mensaje en ASCII
initModule("AT+CNMI=2,2,0,0,0","OK",1000); //Set device to read SMS if available and print to serial // configurar el modulo para leer los mensajes y mostrarl por la consola
Serial.println("Initialized Successfully"); 
  
load1 = EEPROM.read(1); // se le asigna los valores a load segun el valor almacenado en memoria
load2 = EEPROM.read(2);
load3 = EEPROM.read(3);
load4 = EEPROM.read(4);

relays(); // se le asigna los valores a las salidas "reles"  segun los valores almacenados en la eeprom

delay(100);
}

void loop(){
  
  // scan for data from software serial port
  //-----------------------------------------------
  RxString = "";
  Counter = 0;
  while(GSM.available()){ // mientras gsm esta avaliable, realizar la sigiente accion 
    delay(1);  // short delay to give time for new data to be placed in buffer
    // get new character
    RxChar = char(GSM.read()); // se lee un caracter a la vez y es concatenado en una variable string para poder obtener los primeros 200 caracteres juntos y poder evaluar el mensaje recibido
    //add first 200 character to string
    if (Counter < 200) {
      RxString.concat(RxChar);
      Counter = Counter + 1;
    }
  }
 
  // Is there a new SMS?
  //-----------------------------------------------
  if (Received(F("CMT:")) ) GetSMS(); // se verifica si en el string se encuentra la coincidencia F("CMT:")  entonces procede con la captura del mensaje recibido

if(GSM_Nr==phone_no1 || GSM_Nr==phone_no2){ // se evalua que el mensaje solo sea de alguno de estos dos numeros almacenados
  
if(GSM_Msg=="load1on") {load1=0; sendSMS(GSM_Nr,"Ok Load 1 is On");}  // si el mesaje recibido coincide con las palabras "Load1on" entonces cambia el estado de load1 a 0 y devuelve un mensaje de texto confirmando el cambiode esatdo
if(GSM_Msg=="load1off"){load1=1; sendSMS(GSM_Nr,"Ok Load 1 is Off");}

if(GSM_Msg=="load2on") {load2=0; sendSMS(GSM_Nr,"Ok Load 2 is On");}
if(GSM_Msg=="load2off"){load2=1; sendSMS(GSM_Nr,"Ok Load 2 is Off");}

if(GSM_Msg=="load3on") {load3=0; sendSMS(GSM_Nr,"Ok Load 3 is On");}
if(GSM_Msg=="load3off"){load3=1; sendSMS(GSM_Nr,"Ok Load 3 is Off");}

if(GSM_Msg=="load4on") {load4=0; sendSMS(GSM_Nr,"Ok Load 4 is On");}
if(GSM_Msg=="load4off"){load4=1; sendSMS(GSM_Nr,"Ok Load 4 is Off");}

if(GSM_Msg=="allon") {load1=0, load2=0, load3=0, load4=0; sendSMS(GSM_Nr,"Ok All Load is On");} // este comando enciende o apaga todas las salidas
if(GSM_Msg=="alloff"){load1=1, load2=1, load3=1, load4=1; sendSMS(GSM_Nr,"Ok All Load is Off");}

if(GSM_Msg=="loadstatus"){  // si el mensaje coincide con esta palabra, envia un mensaje de texto con el estado actual de todas las salidas
String loadst = "";

if(load1==0){loadst="Load1 On\r\n";}
        else{loadst="Load1 Off\r\n";}
        
if(load2==0){loadst=loadst + "Load2 On\r\n";}
        else{loadst=loadst + "Load2 Off\r\n";}
        
if(load3==0){loadst=loadst + "Load3 On\r\n";}
        else{loadst=loadst + "Load3 Off\r\n";}
        
if(load4==0){loadst=loadst + "Load4 On";}
        else{loadst=loadst + "Load4 Off";}
        
sendSMS(GSM_Nr,loadst); // invocando la funcion para enviar el mesaje final con la respuesta de la operacion realizada
}

eeprom_write(); // invocando funcion para almacenar en memoria los cambios realizados a las variables de salida
relays(); // invocando funcion para aignar los nuevos valores a los pines de salida corespondientes a los relés
}


GSM_Nr="";
GSM_Msg="";
}

void eeprom_write(){ // funcion para alamcenar en memoria los cambios realizados a las variables load
EEPROM.write(1,load1);
EEPROM.write(2,load2);
EEPROM.write(3,load3);
EEPROM.write(4,load4);  
}

void relays(){  // funcion para poner en losp ines de salida los nuevos valores de load
digitalWrite(Relay1, load1); 
digitalWrite(Relay2, load2); 
digitalWrite(Relay3, load3); 
digitalWrite(Relay4, load4); 
}

// Send SMS 
void sendSMS(String number, String msg){ // funcion para enviar mensajes de respuesta
GSM.print("AT+CMGS=\"");GSM.print(number);GSM.println("\"\r\n"); //AT+CMGS=”Mobile Number” <ENTER> - Assigning recipient’s mobile number
delay(500);
GSM.println(msg); // Message contents
delay(500);
GSM.write(byte(26)); //Ctrl+Z  send message command (26 in decimal).
delay(5000);  
}

// Get SMS Content
void GetSMS() { // funcion para obtener el numero del mensaje recibido y el mensaje en cuention 
  //Get SMS number
  //================================================ // en esta parte, se le elimina las comillas a la varible para solo obtener el numero
  GSM_Nr  = RxString;
  //get number
  int t1 = GSM_Nr.indexOf('"');
  GSM_Nr.remove(0,t1 + 1);
  t1 = GSM_Nr.indexOf('"');
  GSM_Nr.remove(t1);
   
  // Get SMS message
  //================================================ // en esta parte se elimina las comillas al mensaje recibido para tener un mensaje limpio 
  GSM_Msg = RxString;
  t1 = GSM_Msg.indexOf('"');
  GSM_Msg.remove(0,t1 + 1);
  t1 = GSM_Msg.indexOf('"');
  GSM_Msg.remove(0,t1 + 1);
  t1 = GSM_Msg.indexOf('"');
  GSM_Msg.remove(0,t1 + 1);
  t1 = GSM_Msg.indexOf('"');
  GSM_Msg.remove(0,t1 + 1);
  t1 = GSM_Msg.indexOf('"');
  GSM_Msg.remove(0,t1 + 1);
  t1 = GSM_Msg.indexOf('"');
  GSM_Msg.remove(0,t1 + 1);
  GSM_Msg.remove(0,1);
  GSM_Msg.trim();

Serial.print("Number:"); Serial.println(GSM_Nr); // muestra en panatalla los datos obtenidos
Serial.print("SMS:"); Serial.println(GSM_Msg);
}

// Search for specific characters inside RxString // busca unos caracteres especificos y devuelve falso o verdadero si lo encuentra
boolean Received(String S) {
  if (RxString.indexOf(S) >= 0) return true; else return false;
}


// Init GSM Module // funcion para establecer la comunicacion, configurar y verificar el estado del mosulo gsm
void initModule(String cmd, char *res, int t){
while(1){
    Serial.println(cmd);
    GSM.println(cmd);
    delay(100);
    while(GSM.available()>0){
       if(GSM.find(res)){
        Serial.println(res);
        delay(t);
        return;
       }else{Serial.println("Error");}}
    delay(t);
  }
}
