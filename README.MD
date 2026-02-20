A Raspberry Pi is used to simulate the WWVB signal to set radio controlled "Atomic Clocks".
<video src="https://github.com/user-attachments/assets/4b6c3f0f-6605-4b91-aebe-df69e2da1e5e"></video>
<p align="center">The LED flashes for each bit of the time signal transmitted</p><br>
<p align="center"><img src="/images/WWVB Transmitter Antenna.png"/>
<p align="center">WWVB Antenna</p><br>
The ferrite rod antenna was cannibalized from a cheap WWVB/60KHz Radio Time Receiver Module purchased from AliExpress. The inductance was measured with an LCR meter and then the capacitor was chosen to make the circuit resonate at 60 KHz.
<br>
<br>
Thanks to Anton Poluektov (https://github.com/apoluekt/raspberry-wwvb) for 'wwvb.c'.
