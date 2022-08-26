<Qucs Schematic 0.0.24>
<Properties>
  <View=-126,-60,1275,953,1,0,0>
  <Grid=10,10,1>
  <DataSet=BJT-noise.dat>
  <DataDisplay=BJT-noise.dpl>
  <OpenDisplay=0>
  <Script=BJT-noise.m>
  <RunScript=0>
  <showFrame=0>
  <FrameText0=Title>
  <FrameText1=Drawn By:>
  <FrameText2=Date:>
  <FrameText3=Revision:>
</Properties>
<Symbol>
</Symbol>
<Components>
  <_BJT Q2N2222A_1 1 400 210 8 -12 0 0 "npn" 0 "8.11e-14" 0 "1" 0 "1" 0 "0.5" 0 "0.225" 0 "113" 0 "24" 0 "1.06e-11" 0 "2" 0 "0" 0 "2" 0 "205" 0 "4" 0 "0" 0 "0" 0 "0.137" 0 "0.343" 0 "1.37" 0 "2.95e-11" 0 "0.75" 0 "0.33" 0 "1.52e-11" 0 "0.75" 0 "0.33" 0 "1" 0 "0" 0 "0.75" 0 "0" 0 "0.5" 0 "3.97e-10" 0 "0" 0 "0" 0 "0" 0 "8.5e-08" 0 "26.85" 0 "0" 0 "1" 0 "1" 0 "0" 0 "1" 0 "1" 0 "0" 0 "1.5" 0 "3" 0 "1.11" 0 "26.85" 0 "1" 0>
  <GND * 1 280 310 0 0 0 0>
  <R R5 1 400 130 20 -19 0 1 "4.7k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <IProbe Pr1 1 490 80 -13 13 1 2>
  <Vdc V1 1 680 140 18 -26 0 1 "12 V" 1>
  <GND * 1 680 170 0 0 0 0>
  <C C1 1 220 210 -32 -58 0 0 "0.1 uF" 1 "" 0 "neutral" 0>
  <GND * 1 400 320 0 0 0 0>
  <R R2 1 400 280 19 -20 0 1 "470" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <R R4 1 600 230 16 -19 0 1 "47k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <GND * 1 600 280 0 0 0 0>
  <R R3 1 280 150 15 -20 0 1 "24k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <R R1 1 280 260 17 -20 0 1 "2k" 1 "26.85" 0 "0.0" 0 "0.0" 0 "26.85" 0 "european" 0>
  <Vac V2 1 140 250 -108 -32 0 1 "1000 mV" 1 "4 kHz" 1 "0" 0 "0" 0>
  <GND * 1 140 300 0 0 0 0>
  <C C2 1 550 170 -34 -57 0 0 "0.1 uF" 1 "" 0 "neutral" 0>
  <.NOISE NOISE1 1 160 380 0 75 0 0 "log" 1 "1 Hz" 1 "10 MHz" 1 "100" 1 "v(out)" 1 "V2" 1>
</Components>
<Wires>
  <280 80 280 120 "" 0 0 0 "">
  <280 80 400 80 "" 0 0 0 "">
  <280 180 280 210 "" 0 0 0 "">
  <280 210 370 210 "" 0 0 0 "">
  <400 80 400 100 "" 0 0 0 "">
  <400 160 400 170 "" 0 0 0 "">
  <400 170 400 180 "" 0 0 0 "">
  <400 80 460 80 "" 0 0 0 "">
  <520 80 680 80 "" 0 0 0 "">
  <680 80 680 110 "" 0 0 0 "">
  <250 210 280 210 "" 0 0 0 "">
  <280 210 280 230 "" 0 0 0 "">
  <280 290 280 310 "" 0 0 0 "">
  <400 240 400 250 "" 0 0 0 "">
  <400 310 400 320 "" 0 0 0 "">
  <600 260 600 280 "" 0 0 0 "">
  <140 210 190 210 "in" 130 180 16 "">
  <140 210 140 220 "" 0 0 0 "">
  <140 280 140 300 "" 0 0 0 "">
  <600 170 600 200 "" 0 0 0 "">
  <580 170 600 170 "" 0 0 0 "">
  <400 170 520 170 "" 0 0 0 "">
  <600 170 600 170 "out" 610 140 0 "">
</Wires>
<Diagrams>
  <Rect 60 849 442 219 3 #c0c0c0 1 10 1 1 1 1e+7 1 -9.06587e-6 5e-5 0.0001 1 -1 0.5 1 315 0 225 "" "" "">
	<"ngspice/ac.inoise_spectrum" #0000ff 0 3 0 0 0>
	<"ngspice/ac.onoise_spectrum" #ff0000 0 3 0 0 1>
  </Rect>
  <Tab 330 566 425 190 3 #c0c0c0 1 00 1 0 1 1 1 0 1 1 1 0 1 11 315 0 225 "" "" "">
	<"ngspice/inoise_total" #0000ff 0 3 0 0 0>
	<"ngspice/onoise_total" #0000ff 0 3 0 0 0>
  </Tab>
</Diagrams>
<Paintings>
  <Text 50 -40 12 #000000 0 "Here is a noise analysis exmple. This is BJT \none-stage amplifier. Simulate this circuit and plot\nonoise _ total and inoise _ total variables to obtain\ndependencies of total noise vs. collector resistance.\nNOTE: This Example could be run only with Ngspice">
</Paintings>
