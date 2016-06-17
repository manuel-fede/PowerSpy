<!--
Version: 1.0
Author: Manuel Federanko
Last Updated: 29. Jan. 16
Version: 1.3
-->

|  Port|   I/O| AN/DIG|  Pin#|   Analog/Digital|             Benutzung|                           Note|
|-----:|-----:|------:|-----:|----------------:|---------------------:|------------------------------:|
|   RA0|    IN|     AN|    17|              AN0|      Current Phase IN|                         COMP 2|
|   RA1|    IN|     AN|    18|              AN1|      Voltage Phase IN|                         COMP 1|
|   RA2|    IN|     AN|     1|              AN2|    REF for Curr Phase| input reference for Curr Phase|
|   RA3|   OUT|    DIG|     2|              AN3|         Display Latch|                               |
|   RA4|    IN|    DIG|     3|              AN4|                Button|                               |
|   RA5|     -|      -|     4|                -|PICKit programmer con.|                   master clear|
|   RA6|   OUT|    DIG|    15|                -|            Status LED|                               |
|   RA7|   OUT|    DIG|    16|                -|          Display DATA|                               |
|   RB0|   OUT|    DIG|     6|                -|                   PWM|    generate REF for Curr Phase|
|   RB1|    IN|    DIG|     7|             AN10|          Bluetooth RX|                               |
|   RB2|   OUT|    DIG|     8|              AN9|          Bluetooth TX|                               |
|   RB3|   OUT|    DIG|     9|              AN8|           Display CLK|                               |
|   RB4|    IN|     AN|    10|              AN7|      Voltage Value IN|      will probably not be used|
|   RB5|    IN|     AN|    11|              AN5|      Current Value IN|                        use ADC|
|   RB6|     -|      -|    12|              AN6|PICKit programmer con.|        nur als programmier Pin|
|   RB7|     -|      -|    13|                -|PICKit programmer con.|        nur als programmier Pin|
|   VDD|     -|      -|    14|                -|          Power supply|                               |
|   VSS|     -|      -|     5|                -|          Power supply|                               |
