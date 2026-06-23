// Projeto 2 - Aferidor de temperatura de forno industrial


// LCD no modo 4 bits
sbit LCD_RS at RB4_bit;
sbit LCD_EN at RB5_bit;
sbit LCD_D4 at RD4_bit;
sbit LCD_D5 at RD5_bit;
sbit LCD_D6 at RD6_bit;
sbit LCD_D7 at RD7_bit;

sbit LCD_RS_Direction at TRISB4_bit;
sbit LCD_EN_Direction at TRISB5_bit;
sbit LCD_D4_Direction at TRISD4_bit;
sbit LCD_D5_Direction at TRISD5_bit;
sbit LCD_D6_Direction at TRISD6_bit;
sbit LCD_D7_Direction at TRISD7_bit;


// Variaveis globais
volatile unsigned int tempo_restante = 60;
volatile unsigned short modo = 1;          // 1 = longo, 2 = curto
volatile unsigned short rodando = 0;
volatile unsigned int cont_250ms = 0;

unsigned int temp_deci = 0;


// Prototipos
void configura_portas(void);
void configura_adc(void);
void configura_lcd(void);
void configura_timers(void);
void configura_interrupcoes(void);

void iniciar_contagem(void);
void ler_temperatura(void);
void atualiza_lcd(void);

void texto_temperatura(unsigned int valor, char *txt);
void texto_tempo(unsigned int valor, char *txt);


// Interrupcoes
void interrupt() {

    // INT0 - botao de selecao do modo
    if (INT0IF_bit) {
        Delay_ms(30);

        // Pull-up: pressionado = 0
        if (PORTB.F0 == 0 && rodando == 0) {
            if (modo == 1) {
                modo = 2;
                tempo_restante = 10;
            } else {
                modo = 1;
                tempo_restante = 60;
            }
        }

        INT0IF_bit = 0;
    }


    // INT1 - botao de inicio
    if (INT1IF_bit) {
        Delay_ms(30);

        // Pull-up: pressionado = 0
        if (PORTB.F1 == 0 && rodando == 0) {
            iniciar_contagem();
        }

        INT1IF_bit = 0;
    }


    // Timer0 - modo longo, aproximadamente 1 segundo
    if (TMR0IF_bit) {
        TMR0H = 0xE1;
        TMR0L = 0x7C;

        if (rodando == 1 && modo == 1 && tempo_restante > 0) {
            tempo_restante--;

            if (tempo_restante == 0) {
                T0CON.TMR0ON = 0;
                rodando = 0;
            }
        }

        TMR0IF_bit = 0;
    }


    // Timer1 - modo curto, aproximadamente 250 ms
    if (TMR1IF_bit) {
        TMR1H = 0x0B;
        TMR1L = 0xDC;

        if (rodando == 1 && modo == 2 && tempo_restante > 0) {
            cont_250ms++;

            if (cont_250ms >= 4) {
                cont_250ms = 0;
                tempo_restante--;

                if (tempo_restante == 0) {
                    T1CON.TMR1ON = 0;
                    rodando = 0;
                }
            }
        }

        TMR1IF_bit = 0;
    }
}


void main() {

    configura_portas();
    configura_adc();
    configura_lcd();
    configura_timers();
    configura_interrupcoes();

    while (1) {
        ler_temperatura();
        atualiza_lcd();

        Delay_ms(200);
    }
}


// Configuracao dos pinos
void configura_portas(void) {

    // AN0 analogico, demais pinos analogicos como digitais
    ADCON1 = 0x0E;

    // PORTA como entrada
    TRISA = 0xFF;

    // PORTD usado pelo LCD
    TRISD = 0x00;

    // Botoes em RB0 e RB1
    TRISB.F0 = 1;
    TRISB.F1 = 1;

    // Pinos de controle do LCD
    TRISB.F4 = 0;
    TRISB.F5 = 0;

    // Limpa saidas do LCD
    PORTB.F4 = 0;
    PORTB.F5 = 0;

    PORTD.F4 = 0;
    PORTD.F5 = 0;
    PORTD.F6 = 0;
    PORTD.F7 = 0;

    // LED em RC0
    TRISC.F0 = 0;
    PORTC.F0 = 0;

    // Habilita pull-up interno do PORTB
    // Mesmo se houver resistor externo, nao atrapalha
    INTCON2.RBPU = 0;
}


// Configuracao do ADC
void configura_adc(void) {

    // AN0 analogico, demais digitais
    // Vref+ = VDD, Vref- = VSS
    ADCON1 = 0x0E;

    // Resultado justificado a direita
    // Fosc/32
    ADCON2 = 0xA2;

    // Canal AN0 selecionado, ADC ligado
    ADCON0 = 0x01;

    Delay_ms(20);
}


// Configuracao do LCD
void configura_lcd(void) {

    Delay_ms(100);

    Lcd_Init();

    Delay_ms(20);

    Lcd_Cmd(_LCD_CLEAR);
    Delay_ms(5);

    Lcd_Cmd(_LCD_CURSOR_OFF);
    Delay_ms(5);

    Lcd_Out(1, 1, "Temp: 00.0 C");
    Lcd_Out(2, 1, "Sel:Long T:060s");
}


// Configuracao dos timers
void configura_timers(void) {

    // Timer0:
    // 16 bits, clock interno, prescaler 1:256
    // Clock de 8 MHz -> aproximadamente 1 segundo
    T0CON = 0x07;
    TMR0H = 0xE1;
    TMR0L = 0x7C;
    TMR0IE_bit = 1;
    T0CON.TMR0ON = 0;


    // Timer1:
    // 16 bits, clock interno, prescaler 1:8
    // Clock de 8 MHz -> aproximadamente 250 ms
    T1CON = 0x30;
    TMR1H = 0x0B;
    TMR1L = 0xDC;
    TMR1IE_bit = 1;
    T1CON.TMR1ON = 0;
}


// Configuracao das interrupcoes
void configura_interrupcoes(void) {

    RCON.IPEN = 0;

    // Pull-up:
    // solto = 1
    // pressionado = 0
    // portanto, interrupcao na borda de descida
    INTEDG0_bit = 0;
    INTEDG1_bit = 0;

    INT0IF_bit = 0;
    INT1IF_bit = 0;

    INT0IE_bit = 1;
    INT1IE_bit = 1;

    TMR0IF_bit = 0;
    TMR1IF_bit = 0;

    PEIE_bit = 1;
    GIE_bit = 1;
}


// Inicia a contagem
void iniciar_contagem(void) {

    rodando = 1;
    cont_250ms = 0;

    if (modo == 1) {
        tempo_restante = 60;

        T1CON.TMR1ON = 0;

        TMR0H = 0xE1;
        TMR0L = 0x7C;
        TMR0IF_bit = 0;

        T0CON.TMR0ON = 1;
    } else {
        tempo_restante = 10;

        T0CON.TMR0ON = 0;

        TMR1H = 0x0B;
        TMR1L = 0xDC;
        TMR1IF_bit = 0;

        T1CON.TMR1ON = 1;
    }
}


// Leitura do ADC e conversao para temperatura
void ler_temperatura(void) {

    unsigned int leitura_adc;

    // Garante canal AN0 selecionado
    ADCON0 = 0x01;

    // Tempo de aquisicao
    Delay_ms(5);

    // Inicia conversao
    ADCON0.GO_DONE = 1;

    // Aguarda terminar
    while (ADCON0.GO_DONE);

    // Resultado de 10 bits justificado a direita
    leitura_adc = ((unsigned int)(ADRESH & 0x03) << 8) | ADRESL;

    temp_deci = (unsigned int)(((unsigned long)leitura_adc * 1000UL) / 204UL);

    if (temp_deci > 1000) {
        temp_deci = 1000;
    }

    // LED acende acima de 50.0 C
    if (temp_deci > 500) {
        PORTC.F0 = 1;
    } else {
        PORTC.F0 = 0;
    }
}


// Atualizacao do LCD
void atualiza_lcd(void) {

    char temp_txt[6];
    char tempo_txt[4];

    texto_temperatura(temp_deci, temp_txt);
    texto_tempo(tempo_restante, tempo_txt);

    // Linha 1: temperatura
    Lcd_Out(1, 1, "Temp:           ");
    Lcd_Out(1, 7, temp_txt);
    Lcd_Out(1, 12, " C");

    // Linha 2: modo e tempo
    if (tempo_restante == 0 && rodando == 0) {
        Lcd_Out(2, 1, "Fim     T:000s  ");
    } else {
        if (modo == 1) {
            Lcd_Out(2, 1, "Sel:Long ");
        } else {
            Lcd_Out(2, 1, "Sel:Curto");
        }

        Lcd_Out(2, 10, " T:");
        Lcd_Out(2, 13, tempo_txt);
        Lcd_Chr(2, 16, 's');
    }
}


// Monta temperatura no formato XX.X
void texto_temperatura(unsigned int valor, char *txt) {

    unsigned int inteiro;
    unsigned int decimal;

    inteiro = valor / 10;
    decimal = valor % 10;

    if (inteiro >= 100) {
        txt[0] = '1';
        txt[1] = '0';
        txt[2] = '0';
        txt[3] = '.';
        txt[4] = '0';
        txt[5] = '\0';
    } else {
        txt[0] = (inteiro / 10) + '0';
        txt[1] = (inteiro % 10) + '0';
        txt[2] = '.';
        txt[3] = decimal + '0';
        txt[4] = '\0';
    }
}


// Monta tempo com 3 digitos
void texto_tempo(unsigned int valor, char *txt) {

    if (valor > 999) {
        valor = 999;
    }

    txt[0] = (valor / 100) + '0';
    txt[1] = ((valor / 10) % 10) + '0';
    txt[2] = (valor % 10) + '0';
    txt[3] = '\0';
}