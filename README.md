# TD-3-LFO
Behringer TD-3 (Roland TB-303  clone)  Cutoff Filter LFO Mod

# What is it.
Arduino modification that allows you to have an in-built LFO controlling the cutoff filter on the TD-3.
Sync'd to the midi-thru clock.
This will also work on the other knobs.

## Specifics
Uses an Arduino (Itsy Bitsy M0 express), which is what I had laying around to drive an op-amp (to get the voltage up) into the filter VCO.
- On/Off toggle switch to bring the LFO in
- Rec button, to record the cutoff filter movement for abitary waveforms and long song changes
- Pot for ajusting amplitude of the LFO
- Pot for adjusting frequency of the LFO
- Timing for frequency comes from the CLK signal on the MIDI thru/Out


## Components
- Arduino (needs to have Analog Out on at least one pin)
- LM 358 op-amp or similar (used to boost the Arduino output up to the VCO 12V)
- 2x Pots
- 1x Momentary Switch
- 1x Toggle Switch
- 2x 10K resistor
- 2x 27k resistor
- Wire, Solder and Soldering Iron  


## How to use
- When playing, press and hold the record button and turn the cutoff filter knob
- Release when you want to stop recording
- Note: After first pressing Rec button, recording ONLY starts when you turn the knob.  When you release the Rec button, the software will find the next best loop point on the beat - so your loop is in time and sounds good.
- Flick the toggle switch so the LFo controls the cutoff filter
- Ajust Amplitude or Frequency (slower and faster) as your like.

