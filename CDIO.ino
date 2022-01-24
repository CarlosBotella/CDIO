#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <ESP8266WiFi.h>


Adafruit_ADS1115 ads1115; // construct an ads1115 at address 0x48

 const int sleepTimeS = 30;
 int cont=0;

  



/*  Serial.println("Posición de humedad");
  while (Serial.available()==0){}
    pHumedad = Serial.parseInt();
    Serial.println("Posición de Salinidad");
  while (Serial.available()==0){delay(1000);}
    pSalinidad = Serial.parseInt();
    Serial.println("Posición de Temperatura");
  while (Serial.available()==0){delay(1000);}
    pTemperatura = Serial.parseInt();
    Serial.println("Posición de Iluminacion");
  while (Serial.available()==0){delay(1000);}
    pLuminidad = Serial.parseInt();  
  
  */
  
  


int power_pin = 5; // GPIO 5 se utiliza para alimentar la sonda de salinidad

// ---------------------------------------------------------------------
// CLASES

class Salinidad{
  private:
    int16_t posicionSalinidad;
    const int SinSal = 200;  // Medimos valor sin sal
    const int ConSal = 5800;  // Medimos valor con sal
    int16_t salinidad;
    int16_t posicion;
  public:
    Salinidad(int16_t posicion){
      posicion = (*this).posicion;}
    int medir(){
      digitalWrite( power_pin, HIGH );
      posicionSalinidad = ads1115.readADC_SingleEnded(posicion);
      digitalWrite(power_pin, LOW);
      salinidad = 100 * SinSal / (SinSal - ConSal) - posicionSalinidad * 100 / (SinSal - ConSal);
      if (salinidad > 100){
        salinidad = 100;}
      if (salinidad < 0){
        salinidad = 0;}
      return salinidad;
     }
};


// R --> Humedad() --> R
class Humedad{
  private:
    int16_t posicionHumedad = ads1115.readADC_SingleEnded(posicion);
    const int AirValue = 29000;  // Medimos valor en seco
    const int WaterValue = 17400;  // Medimos valor en agua
    int16_t humedad;
    int16_t posicion;
  public:
      Humedad(int16_t posicion){
      posicion = (*this).posicion;}
  
    int medir(){
      humedad = 100 * AirValue / (AirValue - WaterValue) - posicionHumedad * 100 / (AirValue - WaterValue);
      if (humedad > 100) {
        humedad = 100;}
    
      // Este "if" impide que la humedad devuelva un valor negativo
      if (humedad < 0) {
        humedad = 0;}
        return humedad;
  }
 };

// R --> Temperatura() --> R
class Temperatura{
  private:
    double temperatura;
    double tension;
    int adc2;
    const double b = 0.786;
    const double m = 0.0347;
    float desviacion;
    int16_t posicion;
  public:
      Temperatura(int16_t posicion){
      posicion = (*this).posicion;}
    int medir(){
      adc2 = (ads1115.readADC_SingleEnded(posicion));// entrada selecionada la 2 comunicanose con nuestra placa
      tension = (4.096 * adc2) / 32767;// factor de conversión para obtener la tension de salida
      desviacion = 2.12;
      temperatura = ((tension - b) / m) + desviacion;// algoritmo para obtener la temperatura (recta de calibracion)
      return temperatura;
    }
  };

 // R --> Iluminacion() --> R
class Iluminacion{
  private:
    int16_t cantIluminacion;
    double formIluminacion;
    int16_t posicion;
  public:
      Iluminacion(int16_t posicion){
      posicion = (*this).posicion;}
    int medir(){
      cantIluminacion = ads1115.readADC_SingleEnded(posicion);
      formIluminacion = cantIluminacion * 100/15000;
      if(formIluminacion > 100){
        formIluminacion=100;}
      if(formIluminacion < 0){
        formIluminacion=0;}
      return formIluminacion;
     }
};



// ---------------------------------------------------------------------


//#include <ESP8266WiFi.h>

// Comentar/Descomentar para ver mensajes de depuracion en monitor serie y/o respuesta del HTTP server
#define PRINT_DEBUG_MESSAGES
//#define PRINT_HTTP_RESPONSE

// Comentar/Descomentar para conexion Fuera/Dentro de UPV
//#define WiFi_CONNECTION_UPV

// Selecciona que servidor REST quieres utilizar entre ThingSpeak y Dweet
#define REST_SERVER_THINGSPEAK //Selecciona tu canal para ver los datos en la web (https://thingspeak.com/channels/360979)
//#define REST_SERVER_DWEET //Selecciona tu canal para ver los datos en la web (http://dweet.io/follow/PruebaGTI)

///////////////////////////////////////////////////////
/////////////// WiFi Definitions /////////////////////
//////////////////////////////////////////////////////

#ifdef WiFi_CONNECTION_UPV //Conexion UPV
  const char WiFiSSID[] = "GTI1";
  const char WiFiPSK[] = "1PV.arduino.Toledo";
#else //Conexion fuera de la UPV
  const char WiFiSSID[] = "Isabel";
  const char WiFiPSK[] = "rhjr7957";
#endif



///////////////////////////////////////////////////////
/////////////// SERVER Definitions /////////////////////
//////////////////////////////////////////////////////

#if defined(WiFi_CONNECTION_UPV) //Conexion UPV
  const char Server_Host[] = "proxy.upv.es";
  const int Server_HttpPort = 8080;
#elif defined(REST_SERVER_THINGSPEAK) //Conexion fuera de la UPV
  const char Server_Host[] = "api.thingspeak.com";
  const int Server_HttpPort = 80;
#else
  const char Server_Host[] = "dweet.io";
  const int Server_HttpPort = 80;
#endif

WiFiClient client;

///////////////////////////////////////////////////////
/////////////// HTTP REST Connection ////////////////
//////////////////////////////////////////////////////

#ifdef REST_SERVER_THINGSPEAK 
  const char Rest_Host[] = "api.thingspeak.com";
  String MyWriteAPIKey="5Q9FFJZZFV6HDXKB"; // Escribe la clave de tu canal ThingSpeak
#else 
  const char Rest_Host[] = "dweet.io";
  String MyWriteAPIKey="cdiocurso2021g02"; // Escribe la clave de tu canal Dweet
#endif

#define NUM_FIELDS_TO_SEND 4 //Numero de medidas a enviar al servidor REST (Entre 1 y 8)

/////////////////////////////////////////////////////
/////////////// Pin Definitions ////////////////
//////////////////////////////////////////////////////

const int LED_PIN = 5; // Thing's onboard, green LED

/////////////////////////////////////////////////////
/////////////// WiFi Connection ////////////////
//////////////////////////////////////////////////////

void connectWiFi()
{
  byte ledStatus = LOW;
  
  #ifdef PRINT_DEBUG_MESSAGES
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
  #endif
  
  WiFi.begin(WiFiSSID, WiFiPSK);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    Serial.println("NO CONECTADO");
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    #ifdef PRINT_DEBUG_MESSAGES
       Serial.println(".");
    #endif
    delay(500);
  }
  #ifdef PRINT_DEBUG_MESSAGES
     Serial.println( "WiFi Connected" );
     Serial.println(WiFi.localIP()); // Print the IP address
  #endif
}

/////////////////////////////////////////////////////
/////////////// HTTP POST  ThingSpeak////////////////
//////////////////////////////////////////////////////

void HTTPPost(String fieldData[], int numFields){

// Esta funcion construye el string de datos a enviar a ThingSpeak mediante el metodo HTTP POST
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar numFields al número adecuado de datos que necesitas enviar y activa los campos en tu canal web

    if (client.connect( Server_Host , Server_HttpPort )){
    
        // Construimos el string de datos. Si tienes multiples campos asegurate de no pasarte de 1440 caracteres
        
        String PostData= "api_key=" + MyWriteAPIKey ;
        for ( int field = 1; field < (numFields + 1); field++ ){
            PostData += "&field" + String( field ) + "=" + fieldData[ field ];
        }     
        
        // POST data via HTTP
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( "Connecting to ThingSpeak for update..." );
        #endif
        client.println( "POST http://" + String(Rest_Host) + "/update HTTP/1.1" );
        client.println( "Host: " + String(Rest_Host) );
        client.println( "Connection: close" );
        client.println( "Content-Type: application/x-www-form-urlencoded" );
        client.println( "Content-Length: " + String( PostData.length() ) );
        client.println();
        client.println( PostData );
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( PostData );
            Serial.println();
            //Para ver la respuesta del servidor
            #ifdef PRINT_HTTP_RESPONSE
              delay(500);
              Serial.println();
              while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
              Serial.println();
              Serial.println();
            #endif
        #endif
    }
}

////////////////////////////////////////////////////
/////////////// HTTP GET  ////////////////
//////////////////////////////////////////////////////

void HTTPGet(String fieldData[], int numFields){

// Esta funcion construye el string de datos a enviar a ThingSpeak o Dweet mediante el metodo HTTP GET
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar "numFields" al número adecuado de datos que necesitas enviar y activa los campos en tu canal web

    if (client.connect( Server_Host , Server_HttpPort )){
           #ifdef REST_SERVER_THINGSPEAK 
              String PostData= "GET https://api.thingspeak.com/update?api_key=5Q9FFJZZFV6HDXKB&field1=0";
              PostData= PostData + MyWriteAPIKey ;
           #else 
              String PostData= "GET http://dweet.io/dweet/for/";
              PostData= PostData + MyWriteAPIKey +"?" ;
           #endif
            
           for ( int field = 1; field < (numFields + 1); field++ ){
              PostData += "&field" + String( field ) + "=" + fieldData[ field ];
           }
            
          
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( "Connecting to Server for update..." );
           #endif
           client.print(PostData);         
           client.println(" HTTP/1.1");
           client.println("Host: " + String(Rest_Host)); 
           client.println("Connection: close");
           client.println();
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( PostData );
              Serial.println();
              //Para ver la respuesta del servidor
              #ifdef PRINT_HTTP_RESPONSE
                delay(500);
                Serial.println();
                while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
                Serial.println();
                Serial.println();
              #endif
           #endif  
    }
}



void setup() {
  ads1115.begin(0x48); //Initialize ads1115
  ads1115.setGain(GAIN_ONE);
  Serial.begin(9600);
  
  #ifdef PRINT_DEBUG_MESSAGES
    Serial.begin(9600);
  #endif
  
  
  digitalWrite(LED_PIN, HIGH);
  
  #ifdef PRINT_DEBUG_MESSAGES
      Serial.print("Server_Host: ");
      Serial.println(Server_Host);
      Serial.print("Port: ");
      Serial.println(String( Server_HttpPort ));
      Serial.print("Server_Rest: ");
      Serial.println(Rest_Host);
  #endif
 }
 
void loop() {
  // Temperatura= 0  Humedad= 1 Salinidad=2  Luz=3
  Temperatura sensor0(0);
  Humedad sensor1(1);
  Salinidad sensor2(2);
  Iluminacion sensor3(3);
  connectWiFi();
String data[4];  // Podemos enviar hasta 8 datos


data[ 1 ] = String(sensor0.medir()); //Escribimos el dato 1. Recuerda actualizar numFields
#ifdef PRINT_DEBUG_MESSAGES
    Serial.print( "Sensor 0 = " );
    Serial.println( data[ 1 ] );
#endif

data[2] = String(sensor1.medir()); //Escribimos el dato 2. Recuerda actualizar numFields
#ifdef PRINT_DEBUG_MESSAGES
    Serial.print( "Sensor 1 = " );
    Serial.println( data[ 2 ] );
#endif

data[3] = String(sensor2.medir()); //Escribimos el dato 3. Recuerda actualizar numFields
#ifdef PRINT_DEBUG_MESSAGES
    Serial.print( "Sensor 2 = " );
    Serial.println( data[ 3 ] );
#endif

data[4] = String(sensor3.medir()); //Escribimos el dato 4. Recuerda actualizar numFields
#ifdef PRINT_DEBUG_MESSAGES
    Serial.print( "Sensor 3 = " );
    Serial.println( data[ 3 ] );
#endif


//Selecciona si quieres enviar con GET(ThingSpeak o Dweet) o con POST(ThingSpeak)
HTTPPost( data, NUM_FIELDS_TO_SEND );
//HTTPGet( data, NUM_FIELDS_TO_SEND );
  //Serial.println(Temperatura(1)); //TODO: Revisar (no es fallo del sensor)
  
  delay(1000);

 /* Serial.print("cont = ");
  Serial.println(cont, DEC);
  if (cont<20){
    cont++;
    delay(1000);
  }
  else{
    Serial.println("ESP8266 in sleep mode");
    ESP.deepSleep(vb  m,.-´´
   sleepTimeS * 100);

}*/
}
