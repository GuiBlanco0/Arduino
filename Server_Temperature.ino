// Inclusão das bibliotecas necessárias
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// === Configurações do Wi-Fi ===
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// === Configurações do Sensor DHT22 ===
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// === Configurações do Display OLED ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
// CORREÇÃO 1: Inicialização correta do display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === Configurações do LED de Alerta ===
#define LED_PIN 2

// === Configurações do Servidor Web ===
WebServer server(80);

// === Variáveis Globais ===
float temperatura = 25.0;  // Valor inicial para não ficar 0
float umidade = 60.0;       // Valor inicial
unsigned long lastTime = 0;
unsigned long timerDelay = 2000;
bool sensorFuncionando = true;
unsigned long lastDisplayUpdate = 0;

// === Página HTML (Interface Web) ===
String paginaWeb() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; margin-top: 50px; background: #1a1a1a; color: white; }";
  html += ".container { max-width: 600px; margin: 0 auto; padding: 20px; background: #2d2d2d; border-radius: 10px; }";
  html += ".temp { font-size: 48px; color: #e67e22; }";
  html += ".humid { font-size: 24px; color: #3498db; }";
  html += ".alerta { color: #e74c3c; font-weight: bold; font-size: 20px; }";
  html += ".normal { color: #27ae60; font-weight: bold; }";
  html += "</style>";
  html += "<title>Monitor do Servidor</title></head><body>";
  html += "<div class='container'>";
  html += "<h1>🌡️ Monitor do Servidor</h1>";
  
  // Mostra status do sensor
  if (sensorFuncionando) {
    html += "<p class='temp'>" + String(temperatura, 1) + " °C</p>";
    html += "<p class='humid'>💧 Umidade: " + String(umidade, 1) + "%</p>";
    
    if (temperatura > 30.0) {
      html += "<p class='alerta'>⚠️ ALERTA: Temperatura ACIMA de 30°C!</p>";
    } else {
      html += "<p class='normal'>✅ Temperatura normal</p>";
    }
  } else {
    html += "<p class='alerta'>⚠️ Sensor com falha!</p>";
    html += "<p>Valores simulados sendo usados:</p>";
    html += "<p class='temp'>" + String(temperatura, 1) + " °C</p>";
    html += "<p class='humid'>💧 Umidade: " + String(umidade, 1) + "%</p>";
  }
  
  html += "<p>⏱️ Última leitura: " + String(millis()/1000) + "s</p>";
  html += "<p><small>IP: " + WiFi.localIP().toString() + "</small></p>";
  html += "</div></body></html>";
  return html;
}

// === Função para ler sensor com fallback ===
void lerSensor() {
  // Tenta ler o sensor
  float novaUmidade = dht.readHumidity();
  float novaTemperatura = dht.readTemperature();
  
  // Verifica se a leitura foi válida
  if (isnan(novaUmidade) || isnan(novaTemperatura)) {
    Serial.println("Falha ao ler sensor! Usando valores simulados...");
    sensorFuncionando = false;
    static float contador = 0;
    contador += 0.5;
    temperatura = 25.0 + sin(contador) * 5;  // Varia entre 20°C e 30°C
    umidade = 60.0 + cos(contador) * 10;      // Varia entre 50% e 70%
  } else {
    sensorFuncionando = true;
    temperatura = novaTemperatura;
    umidade = novaUmidade;
  }
  
  Serial.printf("Temperatura: %.1f °C | Umidade: %.1f %% | Sensor: %s\n", 
                temperatura, umidade, sensorFuncionando ? "OK" : "SIMULADO");

  // Controle do LED de alerta
  digitalWrite(LED_PIN, (temperatura > 30.0) ? HIGH : LOW);
  
  atualizarDisplay();
}

void atualizarDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Cabeçalho
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Servidor");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // Temperatura (grande)
  display.setTextSize(2);
  display.setCursor(0, 15);
  display.print(temperatura, 1);
  
  // Símbolo de grau
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.print("C");
  
  // Umidade
  display.setCursor(0, 40);
  display.setTextSize(1);
  display.print("Umid: ");
  display.print(umidade, 0);
  display.print("%");
  
  // Status do sensor na última linha
  display.setCursor(0, 55);
  if (sensorFuncionando) {
    display.print("Sensor OK");
  } else {
    display.print("Modo simulado");
  }
  
  // Mostra o IP na inicialização (só por alguns segundos)
  static bool mostrouIP = false;
  if (!mostrouIP && millis() < 10000) {  // Primeiros 10 segundos
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("WiFi Conectado!");
    display.println("");
    display.println("IP:");
    display.println(WiFi.localIP());
    display.println("");
    display.println("Acesse no");
    display.println("navegador!");
    mostrouIP = true;
  }
  
  display.display();
}

// === Função para responder às requisições web ===
void handleRoot() {
  server.send(200, "text/html", paginaWeb());
}

// === Setup ===
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== MONITOR DE SERVIDOR ===");
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Teste do LED
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  
  // Inicia o sensor DHT
  dht.begin();
  Serial.println("Sensor DHT iniciado");
  
  // Inicia o Display OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("Falha ao iniciar OLED!");
  } else {
    Serial.println("Display OLED OK");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Iniciando...");
    display.display();
  }
  
  // Conecta ao Wi-Fi
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalha no WiFi! Verifique a rede.");
  }
  
  // Inicia o servidor web
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Servidor HTTP iniciado");
  Serial.println("Acesse: http://" + WiFi.localIP().toString());
}

// === Loop ===
void loop() {
  server.handleClient(); // Processa requisições web
  
  // Lê o sensor a cada 'timerDelay' milissegundos
  if ((millis() - lastTime) > timerDelay) {
    lerSensor();
    lastTime = millis();
  }
  
  // CORREÇÃO 5: Garante que o display atualiza mesmo sem sensor
  if (!sensorFuncionando && (millis() - lastDisplayUpdate) > 1000) {
    atualizarDisplay();
    lastDisplayUpdate = millis();
  }
}