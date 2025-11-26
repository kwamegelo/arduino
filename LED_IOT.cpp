/*
  ESP32 Wi-Fi LED Controller
  Author: Angelo

  Description:
  This project creates a simple web-based LED controller using an ESP32.
  When the system powers on, it connects to a predefined Wi-Fi network.
  Any device on the same network can access the ESP32’s IP address
  to control the LED state (ON / OFF).

  Features:
  - Automatic Wi-Fi connection
  - Local web server hosted on the ESP32
  - Web interface for toggling LED state
  - Status feedback in browser

  Hardware:
  - ESP32 Dev Board
  - LED + 220Ω resistor (connected to GPIO pin)
*/


#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WebServer server(80);

// LED pin definitions
const int LED_PINS[] = {2, 4, 5, 18, 19, 21, 22, 23};
const int NUM_LEDS = sizeof(LED_PINS) / sizeof(LED_PINS[0]);

// PWM properties for brightness control
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8; 

// LED states
struct LEDState {
  bool isOn;
};

LEDState ledStates[8];  

void setup() {
  Serial.begin(115200);
  
  
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    
    
    ledcAttach(LED_PINS[i], PWM_FREQ, PWM_RESOLUTION);
    
    // Initialize LED states
    ledStates[i].isOn = false;
    ledStates[i].brightness = 255;
    ledStates[i].blinkInterval = 0;
    ledStates[i].lastBlinkTime = 0;
    ledStates[i].blinkState = false;
    
    // Turn off all LEDs initially
    ledcWrite(LED_PINS[i], 0);
  }
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Setup web server routes
  setupWebServer();
  
  server.begin();
  Serial.println("Web server started!");
  Serial.println("Open your phone browser and go to: http://" + WiFi.localIP().toString());
}

void loop() {
  server.handleClient();
  handleBlinking();
  delay(10);
}

void setupWebServer() {
  // Serve the main HTML page
  server.on("/", handleRoot);
  
  // API endpoints
  server.on("/api/status", HTTP_GET, handleGetStatus);
  server.on("/api/led", HTTP_POST, handleLEDControl);
  server.on("/api/all", HTTP_POST, handleAllLEDs);
  
  // Handle CORS for API calls
  server.enableCORS(true);
}
void handleRoot() {
  String html = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<title>LED Control System by Angelo</title>";
  html += "<link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">";
  html += "<link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin>";
  html += "<link href=\"https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700;800;900&display=swap\" rel=\"stylesheet\">";
  html += "<style>";
  
  // Enhanced professional color system with modern gradients
  html += ":root{";
  html += "--primary:#3b82f6;--primary-dark:#1e40af;--primary-light:#60a5fa;";
  html += "--success:#10b981;--success-dark:#059669;--success-light:#34d399;";
  html += "--danger:#ef4444;--danger-dark:#dc2626;--danger-light:#f87171;";
  html += "--warning:#f59e0b;--warning-dark:#d97706;--warning-light:#fbbf24;";
  html += "--neutral-50:#fafafa;--neutral-100:#f5f5f5;--neutral-200:#e5e5e5;";
  html += "--neutral-300:#d4d4d4;--neutral-400:#a3a3a3;--neutral-500:#737373;";
  html += "--neutral-600:#525252;--neutral-700:#404040;--neutral-800:#262626;";
  html += "--neutral-900:#171717;";
  html += "--gradient-primary:linear-gradient(135deg,var(--primary) 0%,var(--primary-light) 100%);";
  html += "--gradient-success:linear-gradient(135deg,var(--success) 0%,var(--success-light) 100%);";
  html += "--gradient-danger:linear-gradient(135deg,var(--danger) 0%,var(--danger-light) 100%);";
  html += "--glass-bg:rgba(255,255,255,0.85);";
  html += "--glass-border:rgba(255,255,255,0.2);";
  html += "--shadow-xs:0 1px 2px 0 rgb(0 0 0 / 0.05);";
  html += "--shadow-sm:0 1px 3px 0 rgb(0 0 0 / 0.1), 0 1px 2px -1px rgb(0 0 0 / 0.1);";
  html += "--shadow-md:0 4px 6px -1px rgb(0 0 0 / 0.1), 0 2px 4px -2px rgb(0 0 0 / 0.1);";
  html += "--shadow-lg:0 10px 15px -3px rgb(0 0 0 / 0.1), 0 4px 6px -4px rgb(0 0 0 / 0.1);";
  html += "--shadow-xl:0 20px 25px -5px rgb(0 0 0 / 0.1), 0 8px 10px -6px rgb(0 0 0 / 0.1);";
  html += "--shadow-2xl:0 25px 50px -12px rgb(0 0 0 / 0.25);";
  html += "}";
  
  // Modern reset and base typography
  html += "*{box-sizing:border-box;margin:0;padding:0}";
  html += "body{font-family:'Inter',system-ui,-apple-system,BlinkMacSystemFont,sans-serif;";
  html += "background:linear-gradient(135deg,#f8fafc 0%,#e2e8f0 50%,#cbd5e1 100%);";
  html += "min-height:100vh;color:var(--neutral-800);line-height:1.6;font-weight:400;";
  html += "background-attachment:fixed;-webkit-font-smoothing:antialiased;-moz-osx-font-smoothing:grayscale}";
  
  //container
  html += ".container{max-width:1400px;margin:0 auto;padding:32px 24px;min-height:100vh;";
  html += "display:flex;flex-direction:column;gap:32px}";
  
  //header
  html += ".header{display: flex;align-items:center;padding: 10px 20px;}";

  html += ".company-logo{width:80px;height:80px;margin-right:15px;";
  html += "background:var(--gradient-primary);border-radius:20px;";
  html += "display:flex;align-items:center;justify-content:center;";
  html += "box-shadow:var(--shadow-lg);position:relative}";
  html += ".company-logo::before{content:'⚡';font-size:40px;color:white;filter:drop-shadow(0 2px 4px rgba(0,0,0,0.1))}";
  html += ".company-name{font-size:42px;font-weight:900;color:var(--neutral-900);";
  html += "margin-bottom:12px;letter-spacing:-0.02em;";
  html += "background:linear-gradient(135deg,var(--neutral-900),var(--neutral-600));";
  html += "-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text}";
  html += ".tagline{font-size:14px;color:var(--neutral-500);font-weight:500;";
  html += "text-transform:uppercase;letter-spacing:0.1em;opacity:0.8}";
  
  //status card
  html += ".status-card{display:flex;align-items:center;justify-content:center;";
  html += "transition:all 0.3s cubic-bezier(0.4,0,0.2,1);position:relative;overflow:hidden}";
  html += ".status-card::before{content:'';position:absolute;inset:0;";
  html += "background:linear-gradient(45deg,transparent 30%,rgba(255,255,255,0.1) 50%,transparent 70%);";
  html += "transform:translateX(-100%);transition:transform 0.6s ease;z-index:0}";
  html += ".status-card:hover::before{transform:translateX(100%)}";
  html += ".status-content{display:flex;align-items:center;gap:16px;position:relative;z-index:1}";
  html += ".status-icon{width:16px;height:16px;border-radius:50%;";
  html += "background:var(--danger);flex-shrink:0;position:relative;";
  html += "transition:all 0.3s ease}";
  html += ".status-icon::after{content:'';position:absolute;inset:-4px;";
  html += "border-radius:50%;background:inherit;opacity:0.2;";
  html += "animation:pulse 2s infinite}";
  html += ".status-icon.online{background:var(--success)}";
  html += ".status-icon.warning{background:var(--warning)}";
  html += ".status-text{font-size:18px;font-weight:600;color:var(--neutral-700);";
  html += "transition:color 0.3s ease}";
  html += "@keyframes pulse{0%,100%{transform:scale(1);opacity:0.2}50%{transform:scale(1.2);opacity:0.1}}";
  
  // Premium LED control grid
  html += ".control-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(380px,1fr));";
  html += "gap:24px;align-items:start}";
  
  // Enhanced LED cards with premium styling
  html += ".led-card{background:var(--glass-bg);backdrop-filter:blur(20px);";
  html += "border:1px solid var(--glass-border);border-radius:20px;";
  html += "padding:28px;box-shadow:var(--shadow-md);position:relative;";
  html += "transition:all 0.3s cubic-bezier(0.4,0,0.2,1);overflow:hidden}";
  html += ".led-card::before{content:'';position:absolute;inset:0;";
  html += "background:linear-gradient(135deg,rgba(255,255,255,0.1),transparent);";
  html += "opacity:0;transition:opacity 0.3s ease;z-index:0}";
  html += ".led-card:hover{transform:translateY(-4px);box-shadow:var(--shadow-xl);";
  html += "border-color:rgba(59,130,246,0.3)}";
  html += ".led-card:hover::before{opacity:1}";
  html += ".led-card.active{border-color:var(--success);";
  html += "box-shadow:0 8px 32px -8px rgba(16,185,129,0.3)}";
  html += ".led-card.active::after{content:'';position:absolute;inset:0;";
  html += "background:linear-gradient(135deg,rgba(16,185,129,0.05),transparent);z-index:0}";
  
  // Premium LED card header
  html += ".led-header{display:flex;justify-content:space-between;align-items:center;";
  html += "margin-bottom:24px;position:relative;z-index:1}";
  html += ".led-title{font-size:20px;font-weight:700;color:var(--neutral-900);";
  html += "display:flex;align-items:center;gap:12px}";
  html += ".led-title::before{content:'💡';font-size:18px;opacity:0.7}";
  html += ".led-status{display:flex;align-items:center;gap:10px;";
  html += "padding:8px 16px;border-radius:12px;";
  html += "background:rgba(255,255,255,0.5);backdrop-filter:blur(10px)}";
  html += ".status-dot{width:12px;height:12px;border-radius:50%;";
  html += "background:var(--neutral-300);position:relative;";
  html += "transition:all 0.3s ease}";
  html += ".status-dot::after{content:'';position:absolute;inset:-2px;";
  html += "border-radius:50%;background:inherit;opacity:0.3;";
  html += "transform:scale(0);transition:transform 0.3s ease}";
  html += ".status-dot.on{background:var(--success)}";
  html += ".status-dot.on::after{transform:scale(1.5);animation:ripple 1.5s infinite}";
  html += ".status-label{font-size:13px;font-weight:600;color:var(--neutral-600);";
  html += "text-transform:uppercase;letter-spacing:0.05em}";
  html += "@keyframes ripple{0%{transform:scale(1.5);opacity:0.3}100%{transform:scale(2.5);opacity:0}}";
  
  // Premium control buttons with better separation
  html += ".led-controls{display:grid;grid-template-columns:1fr 1fr;gap:16px;position:relative;z-index:1}";
  html += ".control-btn{padding:16px 24px;border:2px solid;border-radius:12px;";
  html += "font-size:15px;font-weight:700;cursor:pointer;";
  html += "transition:all 0.2s cubic-bezier(0.4,0,0.2,1);";
  html += "text-align:center;text-transform:uppercase;letter-spacing:0.05em;";
  html += "position:relative;overflow:hidden;background:white}";
  html += ".control-btn::before{content:'';position:absolute;inset:0;";
  html += "background:linear-gradient(45deg,transparent 30%,rgba(255,255,255,0.3) 50%,transparent 70%);";
  html += "transform:translateX(-100%);transition:transform 0.6s ease;z-index:0}";
  html += ".control-btn:hover{transform:translateY(-2px);box-shadow:var(--shadow-lg)}";
  html += ".control-btn:hover::before{transform:translateX(100%)}";
  html += ".control-btn:active{transform:translateY(0);transition-duration:0.1s}";
  html += ".control-btn:disabled{opacity:0.5;cursor:not-allowed;transform:none}";
  html += ".control-btn span{position:relative;z-index:1}";
  
  html += ".btn-on{background:var(--gradient-success);border-color:var(--success);color:white}";
  html += ".btn-on:hover{border-color:var(--success-dark);";
  html += "box-shadow:0 8px 25px -8px rgba(16,185,129,0.4)}";
  html += ".btn-off{background:var(--gradient-danger);border-color:var(--danger);color:white}";
  html += ".btn-off:hover{border-color:var(--danger-dark);";
  html += "box-shadow:0 8px 25px -8px rgba(239,68,68,0.4)}";
    
  // Premium master control buttons with better spacing
  html += ".master-controls{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));";
  html += "gap:24px;max-width:600px;margin:0 auto;position:relative;z-index:1}";
  html += ".master-btn{padding:20px 40px;border:3px solid;border-radius:16px;";
  html += "font-size:18px;font-weight:800;cursor:pointer;";
  html += "transition:all 0.25s cubic-bezier(0.4,0,0.2,1);";
  html += "text-transform:uppercase;letter-spacing:0.1em;position:relative;overflow:hidden;";
  html += "background:white;box-shadow:var(--shadow-md)}";
  html += ".master-btn::before{content:'';position:absolute;inset:0;";
  html += "background:linear-gradient(45deg,transparent 30%,rgba(255,255,255,0.4) 50%,transparent 70%);";
  html += "transform:translateX(-100%);transition:transform 0.6s ease;z-index:0}";
  html += ".master-btn:hover{transform:translateY(-3px);box-shadow:var(--shadow-2xl)}";
  html += ".master-btn:hover::before{transform:translateX(100%)}";
  html += ".master-btn:active{transform:translateY(-1px);transition-duration:0.1s}";
  html += ".master-btn:disabled{opacity:0.6;cursor:not-allowed;transform:none}";
  html += ".master-btn span{position:relative;z-index:1;display:flex;align-items:center;justify-content:center;gap:8px}";
  
  html += ".master-on{background:var(--gradient-success);border-color:var(--success);color:white}";
  html += ".master-on:hover{border-color:var(--success-dark);";
  html += "box-shadow:0 16px 40px -12px rgba(16,185,129,0.4)}";
  html += ".master-on span::before{content:'⚡'}";
  html += ".master-off{background:var(--gradient-danger);border-color:var(--danger);color:white}";
  html += ".master-off:hover{border-color:var(--danger-dark);";
  html += "box-shadow:0 16px 40px -12px rgba(239,68,68,0.4)}";
  html += ".master-off span::before{content:'⏹️'}";
  
  // Premium footer
  html += ".footer{text-align:center;padding:32px;";
  html += "color:var(--neutral-500);font-size:15px;font-weight:500;";
  
  // Enhanced responsive design
  html += "@media (max-width:1024px){";
  html += ".container{padding:24px 16px;gap:24px}";
  html += ".control-grid{grid-template-columns:repeat(auto-fill,minmax(320px,1fr));gap:20px}";
  html += "}";
  
  html += "@media (max-width:768px){";
  html += ".container{padding:16px;gap:20px}";
  html += ".header{padding:32px 24px}";
  html += ".company-name{font-size:32px}";
  html += ".company-logo{width:64px;height:64px}";
  html += ".company-logo::before{font-size:32px}";
  html += ".control-grid{grid-template-columns:1fr;gap:16px}";
  html += ".led-card{padding:24px}";
  html += ".master-section{padding:28px}";
  html += ".master-controls{grid-template-columns:1fr;gap:16px}";
  html += ".master-btn{padding:16px 32px;font-size:16px}";
  html += "}";
  
  html += "@media (max-width:480px){";
  html += ".header{padding:24px 16px}";
  html += ".company-name{font-size:28px}";
  html += ".status-card{padding:24px}";
  html += ".led-controls{gap:12px}";
  html += ".control-btn{padding:14px 20px;font-size:14px}";
  html += ".master-section{padding:20px}";
  html += ".master-btn{padding:14px 24px;font-size:15px}";
  html += "}";
  
  html += "</style></head><body>";
  
  html += "<div class=\"container\">";
  
  // Enhanced professional header
  html += "<header class=\"header\">";
  html += "<div class=\"company-logo\"></div>";
  html += "<h1 class=\"company-name\">LightSync</h1>";
  html += "</header>";
  
  // Enhanced system status
  html += "<div class=\"status-card\" id=\"systemStatus\">";
  html += "<div class=\"status-content\">";
  html += "<div class=\"status-icon\" id=\"statusIcon\"></div>";
  html += "<div class=\"status-text\" id=\"statusText\">Initializing system...</div>";
  html += "</div></div>";
  
  // LED controls grid
  html += "<div class=\"control-grid\" id=\"ledGrid\"></div>";
  
  // Enhanced master controls
  html += "<div class=\"master-section\">";
  html += "<div class=\"master-controls\">";
  html += "<button class=\"master-btn master-on\" onclick=\"controlAllLEDs(true)\"><span>Power All ON</span></button>";
  html += "<button class=\"master-btn master-off\" onclick=\"controlAllLEDs(false)\"><span>Power All OFF</span></button>";
  html += "</div></div>";
  
  // Enhanced footer
  html += "<footer class=\"footer\">";
  html += "© 2025 LightSync • IoT Solutions by Angelo";
  html += "</footer>";
  
  html += "</div>";
  
  // Enhanced JavaScript with better error handling
  html += "<script>";
  html += "let ledStates=[];";
  html += "let isConnected=false;";
  html += "let retryCount=0;";
  html += "const maxRetries=3;";
  
  html += "async function fetchStatus(){";
  html += "try{";
  html += "const response=await fetch('/api/status',{";
  html += "method:'GET',";
  html += "headers:{'Accept':'application/json'},";
  html += "timeout:5000";
  html += "});";
  html += "if(!response.ok)throw new Error(`HTTP ${response.status}: ${response.statusText}`);";
  html += "const data=await response.json();";
  html += "ledStates=data.leds||[];";
  html += "retryCount=0;";
  html += "updateUI();";
  html += "updateStatus(true,'System Online');";
  html += "}catch(error){";
  html += "console.error('Connection error:',error);";
  html += "retryCount++;";
  html += "const message=retryCount>=maxRetries?'Connection Failed - Check Network':'Reconnecting...';";
  html += "updateStatus(false,message);";
  html += "if(retryCount<maxRetries)setTimeout(fetchStatus,2000);";
  html += "}}";
  
  html += "function updateStatus(online,message){";
  html += "const icon=document.getElementById('statusIcon');";
  html += "const text=document.getElementById('statusText');";
  html += "icon.className='status-icon '+(online?'online':retryCount<maxRetries?'warning':'');";
  html += "text.textContent=message;";
  html += "isConnected=online;";
  html += "}";
  
  html += "function updateUI(){";
  html += "const grid=document.getElementById('ledGrid');";
  html += "if(!ledStates.length){grid.innerHTML='<div style=\"grid-column:1/-1;text-align:center;color:var(--neutral-500);font-style:italic;\">No LED channels detected</div>';return;}";
  html += "grid.innerHTML='';";
  html += "ledStates.forEach((led,index)=>{";
  html += "const card=document.createElement('div');";
  html += "card.className='led-card'+(led.isOn?' active':'');";
  html += "card.innerHTML=`";
  html += "<div class=\"led-header\">";
  html += "<div class=\"led-title\">Channel ${index+1}</div>";
  html += "<div class=\"led-status\">";
  html += "<div class=\"status-dot${led.isOn?' on':''}\"></div>";
  html += "<div class=\"status-label\">${led.isOn?'ACTIVE':'INACTIVE'}</div>";
  html += "</div></div>";
  html += "<div class=\"led-controls\">";
  html += "<button class=\"control-btn btn-on\" onclick=\"controlLED(${index},true)\"><span>ON</span></button>";
  html += "<button class=\"control-btn btn-off\" onclick=\"controlLED(${index},false)\"><span>OFF</span></button>";
  html += "</div>`;";
  html += "grid.appendChild(card);";
  html += "});}";
  
  html += "async function controlLED(index,state){";
  html += "if(!isConnected){updateStatus(false,'Not Connected - Cannot Control LEDs');return;}";
  html += "const buttons=document.querySelectorAll(`[onclick*=\"controlLED(${index},\"]:not([disabled])`);";
  html += "buttons.forEach(btn=>btn.disabled=true);";
  html += "try{";
  html += "const response=await fetch('/api/led',{";
  html += "method:'POST',";
  html += "headers:{'Content-Type':'application/json','Accept':'application/json'},";
  html += "body:JSON.stringify({led:index,action:state?'on':'off'}),";
  html += "timeout:3000";
  html += "});";
  html += "if(!response.ok)throw new Error(`Control failed: HTTP ${response.status}`);";
  html += "setTimeout(fetchStatus,200);";
  html += "}catch(error){";
  html += "console.error('LED control error:',error);";
  html += "updateStatus(false,'Control Error - Please Try Again');";
  html += "}finally{";
  html += "buttons.forEach(btn=>btn.disabled=false);";
  html += "}}";
  
  html += "async function controlAllLEDs(state){";
  html += "if(!isConnected){updateStatus(false,'Not Connected - Cannot Control LEDs');return;}";
  html += "const buttons=document.querySelectorAll('.master-btn');";
  html += "const allButtons=document.querySelectorAll('.control-btn');";
  html += "buttons.forEach(btn=>btn.disabled=true);";
  html += "allButtons.forEach(btn=>btn.disabled=true);";
  html += "try{";
  html += "const response=await fetch('/api/all',{";
  html += "method:'POST',";
  html += "headers:{'Content-Type':'application/json','Accept':'application/json'},";
  html += "body:JSON.stringify({action:state?'on':'off'}),";
  html += "timeout:5000";
  html += "});";
  html += "if(!response.ok)throw new Error(`Master control failed: HTTP ${response.status}`);";
  html += "updateStatus(true,`All LEDs ${state?'Activated':'Deactivated'} Successfully`);";
  html += "setTimeout(fetchStatus,300);";
  html += "}catch(error){";
  html += "console.error('Master control error:',error);";
  html += "updateStatus(false,'Master Control Error - Please Try Again');";
  html += "}finally{";
  html += "setTimeout(()=>{";
  html += "buttons.forEach(btn=>btn.disabled=false);";
  html += "allButtons.forEach(btn=>btn.disabled=false);";
  html += "},500);";
  html += "}}";
  
  html += "function initializeSystem(){";
  html += "updateStatus(false,'Connecting to LED Control System...');";
  html += "fetchStatus();";
  html += "setInterval(()=>{if(isConnected||retryCount<maxRetries)fetchStatus();},3000);";
  html += "}";
  
  html += "document.addEventListener('DOMContentLoaded',initializeSystem);";
  html += "document.addEventListener('visibilitychange',()=>{";
  html += "if(!document.hidden&&!isConnected)fetchStatus();";
  html += "});";
  html += "</script></body></html>";
  
  server.send(200, "text/html", html);
}

void handleGetStatus() {
  DynamicJsonDocument doc(1024);
  JsonArray ledsArray = doc.createNestedArray("leds");
  
  for (int i = 0; i < NUM_LEDS; i++) {
    JsonObject led = ledsArray.createNestedObject();
    led["isOn"] = ledStates[i].isOn;
    led["blinkInterval"] = ledStates[i].blinkInterval;
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleLEDControl() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }
  
  DynamicJsonDocument doc(512);
  deserializeJson(doc, server.arg("plain"));
  
  int ledIndex = doc["led"];
  String action = doc["action"];
  int value = doc["value"];
  
  if (ledIndex < 0 || ledIndex >= NUM_LEDS) {
    server.send(400, "application/json", "{\"error\":\"Invalid LED index\"}");
    return;
  }
  
  if (action == "on") {
    turnOnLED(ledIndex);
  } else if (action == "off") {
    turnOffLED(ledIndex);
  } 
  
  server.send(200, "application/json", "{\"success\":true}");
}

void handleAllLEDs() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }
  
  DynamicJsonDocument doc(256);
  deserializeJson(doc, server.arg("plain"));
  
  String action = doc["action"];
  
  if (action == "on") {
    turnOnAllLEDs();
  } else if (action == "off") {
    turnOffAllLEDs();
  }
  
  server.send(200, "application/json", "{\"success\":true}");
}

void turnOnLED(int ledNum) {
  if (ledNum >= 0 && ledNum < NUM_LEDS) {
    ledStates[ledNum].isOn = true;
    ledStates[ledNum].blinkInterval = 0;
    ledcWrite(LED_PINS[ledNum], ledStates[ledNum].brightness);
    Serial.println("LED " + String(ledNum + 1) + " turned ON");
  }
}

void turnOffLED(int ledNum) {
  if (ledNum >= 0 && ledNum < NUM_LEDS) {
    ledStates[ledNum].isOn = false;
    ledStates[ledNum].blinkInterval = 0;
    ledcWrite(LED_PINS[ledNum], 0);
    Serial.println("LED " + String(ledNum + 1) + " turned OFF");
  }
}



void turnOffAllLEDs() {
  for (int i = 0; i < NUM_LEDS; i++) {
    ledStates[i].isOn = false;
    ledStates[i].blinkInterval = 0;
    ledcWrite(LED_PINS[i], 0);
  }
  Serial.println("All LEDs turned OFF");
}

void turnOnAllLEDs() {
  for (int i = 0; i < NUM_LEDS; i++) {
    ledStates[i].isOn = true;
    ledStates[i].blinkInterval = 0;
    ledcWrite(LED_PINS[i], ledStates[i].brightness);
  }
  Serial.println("All LEDs turned ON");

}
