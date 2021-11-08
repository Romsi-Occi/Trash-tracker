// Version 1.0

#include <ESP8266WiFi.h>

///////////////Parameters & Constants/////////////////
// WIFI params
char* WIFI_SSID = "Pixel 5 ~ Romsi";    // Configure here the SSID of your WiFi Network
char* WIFI_PSWD = "12345678"; // Configure here the PassWord of your WiFi Network
int WIFI_DELAY  = 100; //ms

// oneM2M : CSE params
String CSE_IP = "192.168.187.180"; //Configure here the IP Address of your oneM2M CSE
int   CSE_HTTP_PORT = 8080;
String CSE_NAME    = "cse-in";
String CSE_RELEASE = "3"; //Configure here the release supported by your oneM2M CSE
bool ACP_REQUIRED = true; //Configure here whether or not ACP is required controlling access
String ACPID = "";

// oneM2M : resources' params
String DESC_CNT_NAME = "DESCRIPTOR";
String DATA_CNT_NAME = "DATA";
String CMND_CNT_NAME = "COMMAND";
String ACP_NAME = "MYACP";
int TY_ACP  = 1;   
int TY_AE  = 2;   
int TY_CNT = 3; 
int TY_CI  = 4;
int TY_SUB = 23;
String originator = "Undefined";

// HTTP constants
int LOCAL_PORT = 80;
char* HTTP_CREATED = "HTTP/1.1 201 CREATED";
char* HTTP_OK    = "HTTP/1.1 200 OK\r\n";
int REQUEST_TIME_OUT = 5000; //ms
int REQUEST_NR = 0;


//MISC
//int TLT_PIN = D2;  // Adapt it accordding to your wiring 
int LED_RED = D5;  // Adapt it accordding to your wiring. Use BUILTIN_LED for onboarded led
int LED_YELLOW = D6;
int LED_GREEN = D7;
int LED_WHITE = D8;
int TRIG_1 = D1;
int TRIG_2 = D3;
int ECHO_1 = D2;
int ECHO_2 = D4;
#define OFF_STATE  LOW // *** External LED is active at HIGH, while Onboarded LED is active at LOW. 
#define ON_STATE HIGH  // *** Adapat it is according to your config
int SERIAL_SPEED  = 115200;

#define DEBUG

///////////////////////////////////////////

// Global variables
const long interval = 10000;
const unsigned long MEASURE_TIMEOUT = 25000UL; // 25ms = ~8m à 340m/s
const float SOUND_SPEED = 340.0 / 1000;
long currentMillis;

WiFiServer server(LOCAL_PORT);    // HTTP Server (over WiFi). Binded to listen on LOCAL_PORT constant
WiFiClient client0;
WiFiClient client1;
String context = "";        // The targeted actuator
String command = "";        // The received command

// Method for creating an HTTP POST with preconfigured oneM2M headers
// param : url  --> the url path of the targted oneM2M resource on the remote CSE
// param : ty --> content-type being sent over this POST request (2 for ae, 3 for cnt, etc.)
// param : rep  --> the representaton of the resource in JSON format
String doPOST(String url, String originator1, int ty, String rep) {

  String relHeader = "";
  if (CSE_RELEASE != "1") {
    relHeader = "X-M2M-RVI: " + CSE_RELEASE + "\r\n";
  }
  String postRequest = String() + "POST " + url + " HTTP/1.1\r\n" +
                       "Host: " + CSE_IP + ":" + CSE_HTTP_PORT + "\r\n" +
                       "X-M2M-Origin: " + originator + "\r\n" +
                       "Content-Type: application/vnd.onem2m-res+json;ty=" + ty + "\r\n" +
                       "Content-Length: " + rep.length() + "\r\n" +
                       relHeader +
                       "X-M2M-RI: req"+REQUEST_NR+"\r\n" +
                       "Connection: close\r\n\r\n" +
                       rep;

  // Connect to the CSE address
  Serial.println("connecting to " + CSE_IP + ":" + CSE_HTTP_PORT + " ...");

  // Get a client
  if (!client1.connect(CSE_IP, CSE_HTTP_PORT)) {
    Serial.println("Connection failed !");
    return "error";
  }

  // if connection succeeds, we show the request to be send
#ifdef DEBUG
  Serial.println(postRequest);
#endif

  // Send the HTTP POST request
  client1.print(postRequest);

  //Update request number after each sending
  REQUEST_NR += 1;
  
  // Manage a timeout
  unsigned long startTime = millis();
  while (client1.available() == 0) {
    if (millis() - startTime > REQUEST_TIME_OUT) {
      Serial.println("Client Timeout");
      client1.stop();
      return "error";
    }
  }

  // If success, Read the HTTP response
  String result = "";
  client1.setTimeout(500);
  if (client1.available()) {
    result = client1.readStringUntil('\r');
    Serial.println(result);
  }
  while (client1.available()) {
    String line = client1.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection...");
  return result;
}

// Method for creating an ApplicationEntity(AE) resource on the remote CSE (this is done by sending a POST request)
// param : ae --> the AE name (should be unique under the remote CSE)
String createAE(String ae) {
  String srv = "";
    if(CSE_RELEASE != "1"){
      srv = ",\"srv\":[\""+CSE_RELEASE+"\"]";
    }
  String aeRepresentation =
    "{\"m2m:ae\": {"
    "\"rn\":\"" + ae + "\","
    "\"api\":\"Norg.demo." + ae + "\","
    "\"rr\":true,"
    "\"poa\":[\"http://" + WiFi.localIP().toString() + ":" + LOCAL_PORT + "/" + ae + "\"]" +
	  srv +
    "}}";
#ifdef DEBUG
  Serial.println(aeRepresentation);
#endif
  return doPOST("/" + CSE_NAME, originator, TY_AE, aeRepresentation);
}

// Method for creating an Access Control Policy(ACP) resource on the remote CSE under a specific AE (this is done by sending a POST request)
// param : ae --> the targeted AE name (should be unique under the remote CSE)
// param : acp  --> the ACP name to be created under this AE (should be unique under this AE)
String createACP(String ae, String acp) {
  String acpRepresentation =
    "{\"m2m:acp\": {"
	"\"rn\":\"" + acp + "\","
	"\"pv\":{\"acr\":[{\"acor\":[\"all\"],\"acop\":63}]},"
	"\"pvs\":{\"acr\":[{\"acor\":[\"all\"],\"acop\":63}]}"
	"}}";
  return doPOST("/" + CSE_NAME + "/" + ae, originator, TY_ACP, acpRepresentation);
}

// Method for creating an Container(CNT) resource on the remote CSE under a specific AE (this is done by sending a POST request)
// param : ae --> the targeted AE name (should be unique under the remote CSE)
// param : cnt  --> the CNT name to be created under this AE (should be unique under this AE)
String createCNT(String ae, String cnt) {
  String cntRepresentation =
    "{\"m2m:cnt\": {"
    "\"mni\":10,"         // maximum number of instances
    "\"rn\":\"" + cnt + "\"" +
	  ACPID + //IF ACP created, it is associated to the container so that anyone has access 
    "}}";
  return doPOST("/" + CSE_NAME + "/" + ae, originator, TY_CNT, cntRepresentation);
}

// Method for creating an ContentInstance(CI) resource on the remote CSE under a specific CNT (this is done by sending a POST request)
// param : ae --> the targted AE name (should be unique under the remote CSE)
// param : cnt  --> the targeted CNT name (should be unique under this AE)
// param : ciContent --> the CI content (not the name, we don't give a name for ContentInstances)
String createCI(String ae, String cnt, String ciContent) {
  String ciRepresentation =
    "{\"m2m:cin\": {"
    "\"con\":\"" + ciContent + "\""
    "}}";
  return doPOST("/" + CSE_NAME + "/" + ae + "/" + cnt, originator,  TY_CI, ciRepresentation);
}


// Method for creating an Subscription (SUB) resource on the remote CSE (this is done by sending a POST request)
// param : ae --> The AE name under which the SUB will be created .(should be unique under the remote CSE)
//          The SUB resource will be created under the COMMAND container more precisely.
String createSUB(String ae) {
  String subRepresentation =
    "{\"m2m:sub\": {"
    "\"rn\":\"SUB_" + ae + "\","
    "\"nu\":[\"" + CSE_NAME + "/" + ae  + "\"], "
    "\"nct\":1,"
    "\"enc\":{\"net\":[3]}"
    "}}";
  return doPOST("/" + CSE_NAME + "/" + ae + "/" + CMND_CNT_NAME, originator,  TY_SUB, subRepresentation);
}


// Method to register a module (i.e. sensor or actuator) on a remote oneM2M CSE
void registerModule(String module, bool isActuator, String intialDescription, String initialData) {
  if (WiFi.status() == WL_CONNECTED) {
    String result;
	
    // 1. Create the ApplicationEntity (AE) for this sensor
    result = createAE(module);
    if (result.equalsIgnoreCase(HTTP_CREATED)) {
      #ifdef DEBUG
      Serial.println("AE " + module + " created  !");
      #endif
	    // 1.1 Create the AccessControlPolicy (ACP) for this sensor so that monitor can subscribe to it
	    if(ACP_REQUIRED) {
		    result = createACP(module, ACP_NAME);
		    if (result.equalsIgnoreCase(HTTP_CREATED)) {
			    ACPID = ",\"acpi\":[\"" + CSE_NAME + "/" + module + "/" + ACP_NAME + "\"]";
          #ifdef DEBUG
			      Serial.println("ACP " + module + " created  !");
          #endif
		    }
	    }
      // 2. Create a first container (CNT) to store the description(s) of the sensor
      result = createCNT(module, DESC_CNT_NAME);
      if (result.equalsIgnoreCase(HTTP_CREATED)) {
        #ifdef DEBUG
        Serial.println("CNT " + module + "/" + DESC_CNT_NAME + " created  !");
        #endif
        // Create a first description under this container in the form of a ContentInstance (CI)
        result = createCI(module, DESC_CNT_NAME, intialDescription);
        if (result.equalsIgnoreCase(HTTP_CREATED)) {
          #ifdef DEBUG
          Serial.println("CI " + module + "/" + DESC_CNT_NAME + "/{initial_description} created !");
          #endif
        }
      }
      // 3. Create a second container (CNT) to store the data  of the sensor
      result = createCNT(module, DATA_CNT_NAME);
      if (result.equalsIgnoreCase(HTTP_CREATED)) {
        #ifdef DEBUG
        Serial.println("CNT " + module + "/" + DATA_CNT_NAME + " created !");
        #endif
        // Create a first data value under this container in the form of a ContentInstance (CI)
        result = createCI(module, DATA_CNT_NAME, initialData);
        if (result.equalsIgnoreCase(HTTP_CREATED)) {
          #ifdef DEBUG
          Serial.println("CI " + module + "/" + DATA_CNT_NAME + "/{initial_data} created !");
          #endif
        }
      }

      // 4. if the module is an actuator, create a third container (CNT) to store the received commands
      if (isActuator) {
        result = createCNT(module, CMND_CNT_NAME);
        if (result.equalsIgnoreCase(HTTP_CREATED)) {
          #ifdef DEBUG
          Serial.println("CNT " + module + "/" + CMND_CNT_NAME + " created !");
          #endif
          // subscribe to any command put in this container
          result = createSUB(module);
          if (result.equalsIgnoreCase(HTTP_CREATED)) {
            #ifdef DEBUG
            Serial.println("SUB " + module + "/" + CMND_CNT_NAME + "/SUB_" + module + " created !");
            #endif
          }
        }
      }
    }
  }
}




void init_IO() {
  // Configure pin 0 for LED control
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_WHITE, OUTPUT);
  pinMode(TRIG_1, OUTPUT);
  pinMode(TRIG_2, OUTPUT);
  pinMode(ECHO_1, INPUT);
  pinMode(ECHO_2, INPUT);
  digitalWrite(LED_RED, OFF_STATE);
  digitalWrite(LED_YELLOW, OFF_STATE);
  digitalWrite(LED_GREEN, OFF_STATE);
  digitalWrite(LED_WHITE, OFF_STATE);
}
void task_IO() {
}


//////////////////////WIFI//////////////////////

void init_WiFi() {
  Serial.println("Connecting to  " + String(WIFI_SSID) + " ...");
  WiFi.persistent(false);
  WiFi.begin(WIFI_SSID, WIFI_PSWD);

  // wait until the device is connected to the wifi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(WIFI_DELAY);
    Serial.print(".");
  }

  // Connected, show the obtained ip address
  Serial.println("WiFi Connected ==> IP Address = " + WiFi.localIP().toString());
}
void task_WiFi() {
}


//////////////////////HTTP SERVER//////////////////////

void init_HTTPServer() {
  server.begin();
  Serial.println("Local HTTP Server started !");
}
void task_HTTPServer() {
  // Check if a client is connected
  client0 = server.available();
  if (client0){  
    // Wait until the client sends some data
    Serial.println("New client connected. Receiving request <=== ");
  
    while (!client0.available()) {
      #ifdef DEBUG
      Serial.print(".");
      #endif
      delay(5);
    }

    // Read the request
    client0.setTimeout(500);
    String request = client0.readString();
    Serial.println(request);

    int start, end;
    // identify the right module (sensor or actuator) that received the notification
    // the URL used is ip:port/ae
    start = request.indexOf("/");
    end = request.indexOf("HTTP") - 1;
    context = request.substring(start+1, end);
    #ifdef DEBUG
    Serial.println(String() + "context = [" + start + "," + end + "] -> " + context);
    #endif

    // ingore verification messages 
    if (request.indexOf("vrq") > 0) {
      client0.flush();
      client0.print(HTTP_OK);
      delay(5);
      client0.stop();
      Serial.println("Client disconnected");
      return;
    }

    //Parse the request and identify the requested command from the device
    //Request should be like "[operation_name]"
    start = request.indexOf("[");  
    end = request.indexOf("]"); // first occurence fo 
    command = request.substring(start+1, end);
    #ifdef DEBUG
    Serial.println(String() + + "command = [" +  start + "," + end + "] -> " + command);
    #endif

    client0.flush();
    client0.print(HTTP_OK);
    delay(5);
    client0.stop();
    Serial.println("Client disconnected");
  }
}

//////////////////////PROXIMITY SENSORS//////////////////////

void init_echo1() {
  String initialDescription = "Name = ProximitySensor1 Unit = Float Location = RightEdgeLvl1";
  String initialData = "0";
  originator = "Cae-ProximitySensor1";
  registerModule("ProximitySensor1", false, initialDescription, initialData);
}

void init_echo2() {
  String initialDescription = "Name = ProximitySensor2 Unit = Float Location = RightEdgeLvl2";
  String initialData = "0";
  originator = "Cae-ProximitySensor2";
  registerModule("ProximitySensor2", false, initialDescription, initialData);
}

void task_echo1() {
  long sensorValue1;
  int proximity1;
  String ciContent = "";
  digitalWrite(TRIG_1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_1, LOW);
  sensorValue1 = pulseIn(ECHO_1, HIGH, MEASURE_TIMEOUT);
  proximity1 = sensorValue1 / 2.0 * SOUND_SPEED;
  #ifdef DEBUG
  Serial.print("Proximity1 ====> ");
  Serial.println(proximity1);
  delay(100);
  #endif
  ciContent = String(proximity1);
  originator = "Cae-ProximitySensor1";
  createCI("ProximitySensor1", DATA_CNT_NAME, ciContent);
}

void command_echo1(String cmd) {
}

void task_echo2() {
  long sensorValue2;
  int proximity2;
  String ciContent = "";
  digitalWrite(TRIG_2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_2, LOW);
  sensorValue2 = pulseIn(ECHO_2, HIGH, MEASURE_TIMEOUT);
  proximity2 = sensorValue2 / 2.0 * SOUND_SPEED;
  #ifdef DEBUG
  Serial.print("Proximity2 ====> ");
  Serial.println(proximity2);
  delay(100);
  #endif
  ciContent = String(proximity2);
  originator = "Cae-ProximitySensor2";
  createCI("ProximitySensor2", DATA_CNT_NAME, ciContent);
}

void command_echo2(String cmd) {
}


//////////////////////LED STATUS//////////////////////

void init_led_red() {
  String initialDescription = "Name = LedFull Location = Box";
  String initialData = "switchOff";
  originator = "Cae-LedFull";
  registerModule("LedFull", true, initialDescription, initialData);
}

void init_led_yellow() {
  String initialDescription = "Name = LedHalf Location = Box";
  String initialData = "switchOff";
  originator = "Cae-LedHalf";
  registerModule("LedHalf", true, initialDescription, initialData);
}

void init_led_green() {
  String initialDescription = "Name = LedEmpty Location = Box";
  String initialData = "switchOff";
  originator = "Cae-LedEmpty";
  registerModule("LedEmpty", true, initialDescription, initialData);
}

void init_led_white() {
  String initialDescription = "Name = LedHeavy Location = Box";
  String initialData = "switchOff";
  originator = "Cae-LedHeavy";
  registerModule("LedHeavy", true, initialDescription, initialData);
}

void task_led_red(String cmd) {
  originator = "Cae-LedFull";
  if (cmd == "switchOn") {
    createCI("LedFull", DATA_CNT_NAME, "1");
  } else if (cmd == "switchOff") {
    createCI("LedFull", DATA_CNT_NAME, "0");
  }  
}

void task_led_yellow(String cmd) {
  originator = "Cae-LedHalf";
  if (cmd == "switchOn") {
    createCI("LedHalf", DATA_CNT_NAME, "1");
  } else if (cmd == "switchOff") {
    createCI("LedHalf", DATA_CNT_NAME, "0");
  }  
}

void task_led_green(String cmd) {
  originator = "Cae-LedEmpty";
  if (cmd == "switchOn") {
    createCI("LedEmpty", DATA_CNT_NAME, "1");
  } else if (cmd == "switchOff") {
    createCI("LedEmpty", DATA_CNT_NAME, "0");
  }  
}

void task_led_white(String cmd) {
  originator = "Cae-LedHeavy";
  if (cmd == "switchOn") {
    createCI("LedHeavy", DATA_CNT_NAME, "1");
  } else if (cmd == "switchOff") {
    createCI("LedHeavy", DATA_CNT_NAME, "0");
  }  
}

void command_led_red(String cmd) {
  if (cmd == "switchOn") {
    #ifdef DEBUG
    Serial.println("Switching on the Full state LED (RED)");
    #endif
    digitalWrite(LED_RED, ON_STATE);
  }
  else if (cmd == "switchOff") {
    #ifdef DEBUG
    Serial.println("Switching off the Full state LED (RED)");
    #endif
    digitalWrite(LED_RED, OFF_STATE);
  }
}

void command_led_yellow(String cmd) {
  if (cmd == "switchOn") {
    #ifdef DEBUG
    Serial.println("Switching on the Half state LED (YELLOW)");
    #endif
    digitalWrite(LED_YELLOW, ON_STATE);
  }
  else if (cmd == "switchOff") {
    #ifdef DEBUG
    Serial.println("Switching off the Half state LED (YELLOW)");
    #endif
    digitalWrite(LED_YELLOW, OFF_STATE);
  }
}

void command_led_green(String cmd) {
  if (cmd == "switchOn") {
    #ifdef DEBUG
    Serial.println("Switching on the Empty state LED (GREEN)");
    #endif
    digitalWrite(LED_GREEN, ON_STATE);
  }
  else if (cmd == "switchOff") {
    #ifdef DEBUG
    Serial.println("Switching off the Empty state LED (GREEN)");
    #endif
    digitalWrite(LED_GREEN, OFF_STATE);
  }
}

void command_led_white(String cmd) {
  if (cmd == "switchOn") {
    #ifdef DEBUG
    Serial.println("Switching on the Heavy state LED (WHITE)");
    #endif
    digitalWrite(LED_WHITE, ON_STATE);
  }
  else if (cmd == "switchOff") {
    #ifdef DEBUG
    Serial.println("Switching off the Heavy state LED (WHITE)");
    #endif
    digitalWrite(LED_WHITE, OFF_STATE);
  }
}

void setup() {
  // intialize the serial liaison
  Serial.begin(SERIAL_SPEED);

  // configure sensors and actuators HW
  init_IO();

  // Connect to WiFi network
  init_WiFi();

  // Start HTTP server
  init_HTTPServer();

  // register sensors and actuators
  init_echo1();
  init_echo2();
  
  init_led_red();
  init_led_yellow();
  init_led_green();
  init_led_white();
}

// Main loop of the µController
void loop() {

  while(millis()-currentMillis < interval) {

    // Check if a client is connected
    task_HTTPServer();
    // analyse the received command (if any)
    if (context != "") {
      if (context == "LedFull") {
        command_led_red(command);
		    task_led_red(command);
      }
      else if (context == "LedHalf") {
        command_led_yellow(command);
        task_led_yellow(command);
      }
      else if (context == "LedEmpty") {
        command_led_green(command);
        task_led_green(command);
      }
      else if (context == "LedHeavy") {
        command_led_white(command);
        task_led_white(command);
      }
      else 
        Serial.println("The target AE does not exist ! ");
    }
    // reset "command" and "context" variables for future received requests
    context = "";
    command = "";
    return;
  }
  currentMillis = millis();
  task_echo1();
  task_echo2();
  
}
