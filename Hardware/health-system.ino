const int pinoSensorIR = 2;      // Pino com interrupção

// Tilt Switch (Posição/Queda)
const int pinoTilt = 3;

// Sensor de Nível (Soro)
const int pinoNivel = 4;

// Buzzer
const int pinoBuzzer = 8;

// LED RGB (Semáforo de Status)
const int pinoLEDvermelho = 5;
const int pinoLEDverde = 6;
const int pinoLEDazul = 7;

// ========== VARIÁVEIS GLOBAIS ==========
// Sistema de Estados
enum EstadoSistema {
  SEGURO,           // Tudo normal - LED VERDE
  ALERTA,           // Alerta médio - LED AMARELO
  ALARME_CRITICO    // Alarme crítico - LED VERMELHO
};

EstadoSistema estadoAtual = SEGURO;

// Variáveis do Sensor IR (Cardíaco)
volatile int batimentos = 0;
unsigned long tempoUltimaLeituraIR = 0;
const unsigned long intervaloLeituraIR = 10000; // 10 segundos
unsigned long ultimoPulsoIR = 0;
const unsigned long intervaloRefratarioIR = 300;
int bpm = 0;
const int BPM_NORMAL_MIN = 50;
const int BPM_NORMAL_MAX = 120;

// Variáveis do Tilt Switch (Posição)
int estadoTiltAnterior = HIGH;
bool quedaDetectada = false;

// Variáveis do Sensor de Nível (Soro)
bool nivelNormal = true;
bool nivelBaixoDetectado = false;

// Variáveis do Buzzer e LED
unsigned long tempoUltimoPisca = 0;
const unsigned long intervaloPiscaAlarme = 500;
const unsigned long intervaloPiscaAlerta = 1000;
bool estadoPisca = false;

// ========== CONFIGURAÇÃO INICIAL ==========
void setup() {
  // Configuração dos pinos
  pinMode(pinoTilt, INPUT);
  pinMode(pinoNivel, INPUT);
  pinMode(pinoBuzzer, OUTPUT);
  pinMode(pinoLEDvermelho, OUTPUT);
  pinMode(pinoLEDverde, OUTPUT);
  pinMode(pinoLEDazul, OUTPUT);
  
  // Configura interrupção para o sensor IR
  attachInterrupt(digitalPinToInterrupt(pinoSensorIR), detectaPulsoIR, RISING);
  
  // Inicializa comunicação serial
  Serial.begin(9600);
  Serial.println("=== SISTEMA DE ALERTA DE SAÚDE INTEGRADO ===");
  Serial.println("Inicializando sensores...");
  
  // Estado inicial seguro (LED Verde)
  atualizarLED(SEGURO);
  noTone(pinoBuzzer);
}

// ========== LOOP PRINCIPAL ==========
void loop() {
  unsigned long tempoAtual = millis();
  
  // 1. ATUALIZA LEITURA DOS SENSORES
  atualizarSensorTilt();
  atualizarSensorNivel();
  atualizarSensorIR(tempoAtual);
  
  // 2. VERIFICA CONDIÇÕES E ATUALIZA ESTADO
  verificarEstadoSistema();
  
  // 3. EXECUTA AÇÕES CONFORME O ESTADO
  executarAcoesEstado(tempoAtual);
  
  // 4. EXIBE STATUS NO MONITOR SERIAL (para debug)
  exibirStatusSerial(tempoAtual);
  
  delay(100); // Pequena pausa para estabilidade
}

// ========== FUNÇÕES DOS SENSORES ==========

// Interrupção para sensor IR (Cardíaco)
void detectaPulsoIR() {
  if (millis() - ultimoPulsoIR > intervaloRefratarioIR) {
    batimentos++;
    ultimoPulsoIR = millis();
  }
}

// Atualiza leitura do sensor IR e calcula BPM
void atualizarSensorIR(unsigned long tempoAtual) {
  if (tempoAtual - tempoUltimaLeituraIR >= intervaloLeituraIR) {
    detachInterrupt(digitalPinToInterrupt(pinoSensorIR));
    
    bpm = batimentos * 6; // Converte para batimentos por minuto
    batimentos = 0;
    tempoUltimaLeituraIR = tempoAtual;
    
    attachInterrupt(digitalPinToInterrupt(pinoSensorIR), detectaPulsoIR, RISING);
  }
}

// Atualiza leitura do sensor de posição
void atualizarSensorTilt() {
  int estadoTiltAtual = digitalRead(pinoTilt);
  
  if (estadoTiltAtual == LOW && estadoTiltAnterior == HIGH) {
    // Transição para posição de queda
    quedaDetectada = true;
    Serial.println("ALARME: Queda/Posição incorreta detectada!");
  } else if (estadoTiltAtual == HIGH && estadoTiltAnterior == LOW) {
    // Retorno à posição normal
    quedaDetectada = false;
    Serial.println("Posição normal restaurada.");
  }
  
  estadoTiltAnterior = estadoTiltAtual;
}

// Atualiza leitura do sensor de nível
void atualizarSensorNivel() {
  bool estadoNivel = digitalRead(pinoNivel);
  
  if (estadoNivel == LOW) { // Sensor NÃO detecta água (nível baixo)
    if (!nivelBaixoDetectado) {
      nivelBaixoDetectado = true;
      Serial.println("ALERTA: Nível de soro baixo!");
    }
    nivelNormal = false;
  } else { // Sensor detecta água (nível normal)
    if (nivelBaixoDetectado) {
      nivelBaixoDetectado = false;
      Serial.println("Nível de soro normalizado.");
    }
    nivelNormal = true;
  }
}

// ========== LÓGICA DE ESTADO ==========

void verificarEstadoSistema() {
  // Hierarquia de prioridades: ALARME_CRITICO > ALERTA > SEGURO
  
  // Condições para ALARME_CRITICO (Máxima prioridade)
  if (quedaDetectada) {
    estadoAtual = ALARME_CRITICO;
    return;
  }
  
  // Condições para ALERTA (Prioridade média)
  if (nivelBaixoDetectado || (bpm > 0 && (bpm < BPM_NORMAL_MIN || bpm > BPM_NORMAL_MAX))) {
    estadoAtual = ALERTA;
    return;
  }
  
  // Condição SEGURO (Estado normal)
  estadoAtual = SEGURO;
}

// ========== AÇÕES DE SAÍDA ==========

void executarAcoesEstado(unsigned long tempoAtual) {
  switch (estadoAtual) {
    case SEGURO:
      // Estado seguro: LED Verde fixo, buzzer desligado
      atualizarLED(SEGURO);
      noTone(pinoBuzzer);
      break;
      
    case ALERTA:
      // Estado de alerta: LED Amarelo piscando lentamente, buzzer intermitente
      atualizarLED(ALERTA);
      executarPisca(tempoAtual, intervaloPiscaAlerta, 600);
      break;
      
    case ALARME_CRITICO:
      // Estado de alarme: LED Vermelho piscando rapidamente, buzzer contínuo
      atualizarLED(ALARME_CRITICO);
      executarPisca(tempoAtual, intervaloPiscaAlarme, 1200);
      break;
  }
}

void atualizarLED(EstadoSistema estado) {
  // Desliga todos os LEDs primeiro
  digitalWrite(pinoLEDvermelho, LOW);
  digitalWrite(pinoLEDverde, LOW);
  digitalWrite(pinoLEDazul, LOW);
  
  switch (estado) {
    case SEGURO:
      digitalWrite(pinoLEDverde, HIGH);  // VERDE
      break;
    case ALERTA:
      digitalWrite(pinoLEDvermelho, HIGH); // VERMELHO +
      digitalWrite(pinoLEDverde, HIGH);    // VERDE = AMARELO
      break;
    case ALARME_CRITICO:
      digitalWrite(pinoLEDvermelho, HIGH); // VERMELHO
      break;
  }
}

void executarPisca(unsigned long tempoAtual, unsigned long intervalo, int frequenciaBuzzer) {
  if (tempoAtual - tempoUltimoPisca >= intervalo) {
    tempoUltimoPisca = tempoAtual;
    estadoPisca = !estadoPisca;
    
    if (estadoPisca) {
      tone(pinoBuzzer, frequenciaBuzzer);
    } else {
      noTone(pinoBuzzer);
    }
  }
}

// ========== MONITORAMENTO SERIAL ==========

void exibirStatusSerial(unsigned long tempoAtual) {
  static unsigned long ultimoPrint = 0;
  
  if (tempoAtual - ultimoPrint >= 2000) { // A cada 2 segundos
    ultimoPrint = tempoAtual;
    
    Serial.println("=== STATUS DO SISTEMA ===");
    Serial.print("Estado: ");
    switch (estadoAtual) {
      case SEGURO: Serial.println("SEGURO (Verde)"); break;
      case ALERTA: Serial.println("ALERTA (Amarelo)"); break;
      case ALARME_CRITICO: Serial.println("ALARME CRÍTICO (Vermelho)"); break;
    }
    
    Serial.print("Frequência Cardíaca: ");
    if (bpm > 0) {
      Serial.print(bpm);
      Serial.println(" BPM");
    } else {
      Serial.println("Aguardando leitura...");
    }
    
    Serial.print("Posição: ");
    Serial.println(quedaDetectada ? "Queda/Incorreta" : "Normal");
    
    Serial.print("Nível de Soro: ");
    Serial.println(nivelBaixoDetectado ? "BAIXO" : "NORMAL");
    
    Serial.println("========================");
  }
}
