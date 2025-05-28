#include <DHT.h>                     // Biblioteca para o sensor DHT (temperatura e umidade)
#include <LiquidCrystal_I2C.h>      // Biblioteca para o display LCD com interface I2C

// Definições do pino e tipo do sensor DHT
#define DHTPIN 2                    // Pino digital ao qual o DHT22 está conectado
#define DHTTYPE DHT22               // Tipo do sensor DHT (modelo DHT22)

DHT dht(DHTPIN, DHTTYPE);           // Inicializa o sensor DHT22
LiquidCrystal_I2C lcd(0x27, 16, 2); // Inicializa o LCD I2C no endereço 0x27 com 16 colunas e 2 linhas

const int sensorNivelPin = A0;      // Pino analógico usado para ler o sensor de nível
const int buzzerPin = 8;            // Pino digital onde o buzzer está conectado

// Definição dos pinos dos LEDs
const int ledVermelho = 13;
const int ledAmarelo = 12;
const int ledVerde = 11;

// Criação de um caractere customizado (uma lanterna) para exibir no LCD
byte lanternaChar[8] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100
};

void setup() {
  Serial.begin(9600);             // Inicializa a comunicação serial a 9600 bps
  dht.begin();                    // Inicia o sensor DHT
  lcd.init();                     // Inicia o LCD
  lcd.backlight();                // Liga a luz de fundo do LCD

  // Define os modos dos pinos
  pinMode(sensorNivelPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVerde, OUTPUT);

  lcd.createChar(0, lanternaChar); // Registra o caractere customizado no LCD
}

void loop() {
  // Leitura do sensor de nível (valor analógico entre 0 e 1023)
  int nivelRaw = analogRead(sensorNivelPin);
  
  // Mapeia o valor lido para um intervalo de 0 a 300 cm
  int nivelCm = map(nivelRaw, 0, 1023, 0, 300);
  float nivelMetros = nivelCm / 100.0; // Converte para metros

  // Leitura de temperatura e umidade do sensor DHT22
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  // Verifica se as leituras são válidas
  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Erro ao ler DHT22");
    return; // Sai do loop se houver erro
  }

  // Exibe os dados no LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Niv:");
  lcd.print(nivelMetros);
  lcd.print("m");

  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temperatura, 1); // 1 casa decimal
  lcd.print("C ");
  lcd.print("U:");
  lcd.print(umidade, 0);     // Sem casas decimais
  lcd.print("%");

  // Exibe os dados também no monitor serial
  Serial.print("Nivel cm: ");
  Serial.print(nivelCm);
  Serial.print(" | Temp: ");
  Serial.print(temperatura);
  Serial.print("C | Umid: ");
  Serial.print(umidade);
  Serial.print("%");

  // Lógica de alerta com base no nível de água
  if (nivelCm > 200) {
    // Alerta de nível alto
    tone(buzzerPin, 1000);               // Liga o buzzer com frequência de 1kHz
    digitalWrite(ledVermelho, HIGH);     // Liga LED vermelho
    digitalWrite(ledAmarelo, LOW);       // Desliga LED amarelo
    digitalWrite(ledVerde, LOW);         // Desliga LED verde

    lcd.setCursor(13, 0);                // Posição no LCD para a lanterna
    lcd.write(byte(0));                  // Exibe a lanterna no LCD

    Serial.println(" | ALERTA: Nivel ALTO!");
  
  } else if (nivelCm > 100) {
    // Nível intermediário
    noTone(buzzerPin);                   // Desliga buzzer
    digitalWrite(ledVermelho, LOW);
    digitalWrite(ledAmarelo, HIGH);
    digitalWrite(ledVerde, LOW);

    Serial.println(" | ATENCAO: Nivel Medio");

  } else {
    // Nível seguro
    noTone(buzzerPin);
    digitalWrite(ledVermelho, LOW);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVerde, HIGH);

    Serial.println(" | Status: Seguro");
  }

  delay(1000); // Espera 1 segundo antes de repetir
}
