- Refs

  - code "triac_timer" de FLorent
  - code ac_dimemr ESPHome: https://github.com/esphome/esphome/tree/dev/esphome/components/ac_dimmer
  - Detecte pulse with RMT + PCNT
  - https://gist.github.com/mathieucarbou
  - https://github.com/mathieucarbou/YaSolR-OSS/discussions/21
  - https://github.com/fabianoriccardi/dimmable-light/issues/58
  - https://github.com/mathieucarbou/YaSolR-OSS/issues/18

- Scripts / Code

  - Test_sortie_triac_et_zero_crosing.ino
  - StochasticHeatController
  - LUT table based on Florent's algo: https://github.com/mathieucarbou/YaSolR-OSS/discussions/21
  - MCPWM:
    - https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/mcpwm.html
    - https://electronics.stackexchange.com/questions/632466/firing-a-triac-with-pwm
    - https://github.com/mathieucarbou/YaSolR-OSS/issues/18#issuecomment-2170098387
    - https://github.com/JoaoLopesF/ESP32MotorControl/blob/master/ESP32MotorControl.cpp
    - https://github.com/espressif/esp-idf/issues/2943#issuecomment-926485532
  - PWM: https://github.com/Dlloydev/ESP32-ESP32S2-AnalogWrite

- formules:

  - https://colab.research.google.com/drive/1U5KORqsfGtoyXcHlmeeCd-Ny1pnk1b7E?usp=sharing#scrollTo=F1Hk-fMTBly7
  - page 13 formules: http://philippe.demerliac.free.fr/RichardK/Graduateur.pdf
  - facteur forme: fdf = 1 / ( sqrt( 1 + thdi ^ 2) ) https://youtu.be/_SipRlVWlqM?list=PL-IXE4AO5wkuxvQLEB-AuwoxZF1ZRzClf&t=1755
  - puissance sortie triac: b1 = 1 - a / pi + sin(2*a) / 2*pi https://youtu.be/_SipRlVWlqM?list=PL-IXE4AO5wkuxvQLEB-AuwoxZF1ZRzClf&t=1944
  - https://youtu.be/uV1YzIjNjCw?t=744
  - https://youtu.be/nOtegLh8FWM?list=PLWpzro3Ndk_2PUlQkULUjP6VSzwmFXkPc&t=1088
  - [HARMONICS: CAUSES, EFFECTS AND MINIMIZATION](https://www.salicru.com/files/pagina/72/278/jn004a01_whitepaper-armonics_%281%29.pdf)
  
- Notes

  - Ref: https://github.com/mathieucarbou/YaSolR-OSS/discussions/21
  - Vrms*dimmer = ActivePower / Irms or Vrms_dimmer = Vrms_grid * PowerFactor or Vrms*dimmer = Vrms_grid * sqrt(power_duty)
  - My system is first determining the dimmer max power once by turning it 0%/100% a few times and by reading the active power from the grid. It is used to compute and store the heating element resistance (R = ((Vrms grid) ^ 2) / ActivePower).
    During the routing, I measure the grid's power, compute the power error + PID, add/remove it from the duty and then lookup an interpolated phase delay.
    My lookup table has 80 unit16 generated from the duty2phase function I posted earlier.
    The error is less then 0.001 with a perfect sine wave.
    The heating element max power is Router*PowerMax = (Grid_Vrms ^ 2) / R
    The reported routed power is Router_Power = duty * Router*PowerMax
    The reported routed voltage is Router_Vrms = Grid_Vrms * sqrt(duty)
    The reported routed current is Router_Irms = Router_Power / Router_Vrms
    The reported routed power factor is Router_PowerFactor = sqrt(duty)
