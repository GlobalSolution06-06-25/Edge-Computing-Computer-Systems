#include <DHT.h>                     // Biblioteca para o sensor de temperatura e umidade
#include <LiquidCrystal_I2C.h>      // Biblioteca para o display LCD com interface I2C

#define DHTPIN 2                    // Pino digital onde o DHT22 está conectado
#define DHTTYPE DHT22               // Define o tipo de sensor (DHT22)

DHT dht(DHTPIN, DHTTYPE);           // Criação do objeto do sensor DHT
LiquidCrystal_I2C lcd(0x27, 16, 2); // Criação do objeto do LCD (endereço 0x27, 16 colunas, 2 linhas)

const int sensorNivelPin = A0;      // Pino analógico do sensor de nível d'água
const int buzzerPin = 8;            // Pino do buzzer

// Pinos dos LEDs indicadores
const int ledVermelho = 13;
const int ledAmarelo = 12;
const int ledVerde = 11;

// Desenho personalizado (símbolo de alerta) para o LCD
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
  Serial.begin(9600);              // Inicializa a comunicação serial
  dht.begin();                     // Inicializa o sensor DHT
  lcd.init();                      // Inicializa o LCD
  lcd.backlight();                 // Liga a luz de fundo do LCD

  pinMode(sensorNivelPin, INPUT); // Define o pino do sensor de nível como entrada
  pinMode(buzzerPin, OUTPUT);     // Define o pino do buzzer como saída

  // Define os pinos dos LEDs como saída
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVerde, OUTPUT);

  lcd.createChar(0, lanternaChar); // Cria o caractere personalizado no LCD
}

void loop() {
  // Leitura do sensor de nível (0–1023), mapeado para 0–300 cm
  int nivelRaw = analogRead(sensorNivelPin);
  int nivelCm = map(nivelRaw, 0, 1023, 0, 300);
  float nivelMetros = nivelCm / 100.0;

  // Leitura da temperatura e umidade do DHT22
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  // Verifica se a leitura do DHT22 falhou
  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Erro ao ler DHT22");
    return;
  }

  // Exibe dados no LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Niv:");
  lcd.print(nivelMetros);
  lcd.print("m");

  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temperatura, 1);
  lcd.print("C ");
  lcd.print("U:");
  lcd.print(umidade, 0);
  lcd.print("%");

  // Envia dados ao monitor serial
  Serial.print("Nivel cm: ");
  Serial.print(nivelCm);
  Serial.print(" | Temp: ");
  Serial.print(temperatura);
  Serial.print("C | Umid: ");
  Serial.print(umidade);
  Serial.print("%");

  // Lógica de alerta com LEDs e buzzer
  if (nivelCm > 200) {
    // Nível muito alto: alerta
    tone(buzzerPin, 1000); // Ativa buzzer com tom de 1 kHz
    digitalWrite(ledVermelho, HIGH);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVerde, LOW);

    lcd.setCursor(13, 0);
    lcd.write(byte(0)); // Mostra o símbolo de alerta no LCD

    Serial.println(" | ALERTA: Nivel ALTO!");
  } else if (nivelCm > 100) {
    // Nível intermediário: atenção
    noTone(buzzerPin);
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

  delay(1000); // Aguarda 1 segundo antes da próxima leitura
}
