bottomSensor

Dieses Programm wertet den Liniensensor des Roboters aus. Der Bodensensor verfügt über 3 analoge Multiplexer welche hier ausgelesen werden. Auf Basis dieser Daten wird dann die Position der Linie berechnet. Bei einer Linienerkennung setzt der Arduino Nano ein Interrupt-Signal am Arduino Mega auf HIGH und sendet die Position der Linie über UART an den Mega. 