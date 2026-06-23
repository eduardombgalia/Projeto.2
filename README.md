# Projeto.2

Gabriel Durães Betinarde - 15676732

Eduardo Marani Barbosa Galia - 15574332


Aferidor de Temperatura e Tempo para Forno Industrial:
Este projeto consiste no desenvolvimento de um sistema embarcado baseado no PIC18F4550 (utilizando a infraestrutura do Kit EasyPIC v7 e simulação no SimulIDE) voltado para o monitoramento térmico e controle de tempo de operação em um forno industrial, integrando periféricos analógicos e digitais operando sob uma arquitetura de firmware em linguagem C.

O dispositivo tem duas funções principais:

  Medição de Temperatura: Realiza a leitura analógica de um sensor LM35 por meio do conversor analógico-digital (ADC) interno de 10 bits. A faixa monitorada abrange  de 0 °C a 100 °C, sendo formatada e exibida no display LCD no padrão XX.X °C sem o uso de variáveis do tipo float, otimizando a memória de dados.
  
  Controle de Tempo: Dispõe de duas modalidades de contagem regressiva para a operação, selecionadas via botão (interface digital tratada contra efeito bouncing):
  
    Modo Longo (60 segundos): Temporizado através do estouro do Timer0, gerando uma base de tempo precisa de 1 segundo.
    Modo Curto (10 segundos): Temporizado através do estouro do Timer1, operando com uma base de clock de 250 ms
Para a exibição do status atual do sistema, foi utilizado um display LCD 16x2 configurado em modo de comunicação serial de 4 bits, mostrando em tempo real o valor térmico na primeira linha, além do modo ativo (Longo/Curto) e a contagem de tempo restante na segunda linha.
Além disso foi implementado uma lógica de acionamento em malha para simular o controle de resistência elétrica do forno por meio de um LED indicador acoplado ao pino RC0.
