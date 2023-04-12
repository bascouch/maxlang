# maxlang API documentation

### Modules

#### **_commontoall_**

- **name**  

```
module name=myfavoritemodule ....
```
  - _unit:_ string
  - _descr:_ define yout own name for this module to be able to address it later.
  you can send parameters directly with **send myfavoritemodule**


---
#### **in**

  audio in from specific adc.

  ```
  in adc=[1,2] # input Clar
  ```

  - **adc**  define adc channel to get audio from
    - _unit:_ array : [integer, integer, ...]
    - _default:_ [1,2]



---
#### **out**

  audio out to specific dac.
  vca features to control volumes of multiples chains.

  ```
  out vca=1 dac=[1,2,3,4]
  ```

  - **dac**  define dac channel to send audio to
    - _unit:_ array : [integer, integer, ...]
    - _default:_ [1,2]

  - **vca**  vca channel bind to this output
    - _unit:_ number : integer
    - _default:_ 1
    - _min:_ 1
    - _max:_ 8

  - **gain** db gain
	  - _unit:_ dB : float
	  - _default:_ 0.
	  - _min:_ none
	  - _max:_ none


---
#### **delay**

variable delay of audio signal. time can be modulated (makes kind of granular mashup)

```
delay name=delayseqC time=rand(freq=randi(freq=10,min=0.1,max=10,curve=1)
  ,min=0,max=5000,curve=1)

```

- **time** delay time in ms
  - _unit:_ time ms : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ 60000.

- **feedback** cross feedback in % of the delay line
  - _unit:_ % : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ 100.

- **drywet** crossfade in % between dry and wet signal
  - _unit:_ % : float
  - _default:_ 100.
  - _min:_ 0.
  - _max:_ 100.

- **gain** gain in dB
  - _unit:_ dB : float
  - _default:_ 0.
  - _min:_ none
  - _max:_ none


  ---
  #### **multidelay**

  bunch of 12 delays with random distribution between _time_ and _time_ + _timeframe_. _timecurve_ sets the time distribution in the time frame. _ampdecay_ set the amp distribution of the delay taps.

  ```
  multidelay drywet=50 feedback=50 time=100 timeframe=1000 ampdecay=10 seed=CBA


  ```

  - **ingain** gain in dB of the input signal feeding the delay.
    - _unit:_ dB : float
    - _default:_ 0.
    - _min:_ none
    - _max:_ none


  - **time** first delay time in ms
    - _unit:_ time ms : float
    - _default:_ 200.
    - _min:_ 0.
    - _max:_ 60000.

  - **timeframe** duration between first and last delay tap
    - _unit:_ time ms : float
    - _default:_ 1000.
    - _min:_ 0.
    - _max:_ 60000.

  - **timecurve** time distribution of the random delay taps. 0 means equally distributed delays. 1.5 means delays are generally closer to start time than to end time.
    - _unit:_ curve : float
    - _default:_ 0.
    - _min:_ -3.
    - _max:_ 3.

  - **ampdecay** amp distribution (if you want the delay amp to decay)
    - _unit:_  float
    - _default:_ 0.
    - _min:_ 0.
    - _max:_ 100.


  - **feedback** cross feedback in % of the delay line
    - _unit:_ % : float
    - _default:_ 0.
    - _min:_ 0.
    - _max:_ 100.

  - **seed** 3 letters to seed the random generator. ex: ABC, ADA, ZOU. A the delay random distribution will always be the same when you specify a specific seed. The default is no seed, means it will be always different.
    - _unit:_ 3 LETTERS (ABA,...)
    - _default:_ none


  - **drywet** crossfade in % between dry and wet signal
    - _unit:_ % : float
    - _default:_ 100.
    - _min:_ 0.
    - _max:_ 100.

  - **gain** gain in dB
    - _unit:_ dB : float
    - _default:_ 0.
    - _min:_ none
    - _max:_ none



---
#### **filter**

  multimode butterworth filter. not meant to be modulated but allowed.

  ```
  filter name=filterseqC freq=220 mode=3 width=12

  ```

  - **freq** central frequency of the filter
    - _unit:_ Hz : float
    - _default:_ 200.
    - _min:_ 20.
    - _max:_ 20000.

  - **mode** mode of filter 0:lowpass 1:highpass 2:bandpass 3:bandstop
    - _unit:_ type : int
    - _default:_ 1 (highpass)
    - _min:_ 0
    - _max:_ 3

  - **width** bandwidth for bandpass and bandstop mode in semitones
    - _unit:_ semitones : float
    - _default:_ 12.
    - _min:_ 0.1
    - _max:_ none

- **gain** gain in dB
  - _unit:_ dB : float
  - _default:_ 0.
  - _min:_ none
  - _max:_ none


---
#### **disto**

  Cubic nonlinearity distortion. ( Faust version )



  ```
  disto drive=0.3 offset=0.01 drywet=50

  ```

  - **drive** drive amount
    - _unit:_ amount : float
    - _default:_ 0.
    - _min:_ 0.
    - _max:_ 1.

  - **offset** constant added before nonlinearity to give even harmonics.  
    - _unit:_ value : float
    - _default:_ 0.
    - _min:_ -4.
    - _max:_ 4.

  - **drywet** crossfade between dry and wet signal
    - _unit:_ % : float
    - _default:_ 100.
    - _min:_ 0.
    - _max:_ 100.

- **gain** gain in dB
  - _unit:_ dB : float
  - _default:_ 0.
  - _min:_ none
  - _max:_ none

---
#### **freqshift**

  frequency shifter of audio signal.

  ```
  freqshift name=shishi shift=randi(freq=0.1, min=-100, max=100, curve=0)

  ```

  - **shift** shift amount in Hz
    - _unit:_ Hz : float
    - _default:_ 0.
    - _min:_ none
    - _max:_ none

  - **drywet** crossfade in % between dry and wet signal
    - _unit:_ % : float
    - _default:_ 100.
    - _min:_ 0.
    - _max:_ 100.

- **gain** gain in dB
  - _unit:_ dB : float
  - _default:_ 0.
  - _min:_ none
  - _max:_ none

---
#### **gizmo**

  transpose audio signal with gizmo~ (fft scaling method).

  ```
  gizmo ratio= 0.2

  ```

  - **ratio** frequency multiplier
    - _unit:_ ratio : float
    - _default:_ 1.
    - _min:_ 0.001
    - _max:_ 64.

- **gain** gain in dB
  - _unit:_ dB : float
  - _default:_ 0.
  - _min:_ none
  - _max:_ none

---
#### **pitchshift**

  pitch shifter of audio signal. uses pitchshift~

  ```
  pitchshift transp=-1200.
  ```

  - **transp** pitchshift amount in cents
    - _unit:_ cents : float
    - _default:_ 0.
    - _min:_ none
    - _max:_ none

  - **gain** gain in dB
	  - _unit:_ dB : float
	  - _default:_ 0.
	  - _min:_ none
	  - _max:_ none


---
#### **svptrans**

supervp transposer. pretty amount of sound tweaking between sinus and noise parts.
warning : adds latency (1024 pts ~= 20 ms)

```
svptrans name=svp1 transp=-50. envscale=0.7  sinus=1. error=0.2 gain=6
```

	- **transp** transposition factor in cents  
  - _unit:_ cents : float
  - _default:_ 0.
  - _min:_ none
  - _max:_ none

- **transpadd** second transposition factor to add to _transp_ (allows modulating for example)
  - _unit:_ cents : float
  - _default:_ 0.
  - _min:_ none
  - _max:_ none

- **envscale** frequency scaling of the spectral envelope. sets _envscale.timbre_ and _envscale.mean_
    0.5 : brighter sound
    1.5 : muffled sound
  - _unit:_ coeff : float
  - _default:_ 1.
  - _min:_ 0.
  - _max:_ 2.



- **envscale.timbre** timbre part of envscale
  - _unit:_ coeff : float
  - _default:_ 1.
  - _min:_ 0.
  - _max:_ 2.

- **envscale.mean** mean part of envscale
  - _unit:_ coeff : float
  - _default:_ 1.
  - _min:_ 0.
  - _max:_ 2.

- **sinus**  sinusoidal gain for resynthesis. Use it ratio with _noise_
  - _unit:_ amp lin (pow2) : float
  - _default:_ 1.
  - _min:_ 0.
  - _max:_ 2.

- **noise** noise gain for resynthesis. Use it ratio with _sinus_
  - _unit:_ amp lin (pow2) : float
  - _default:_ 1.
  - _min:_ 0.
  - _max:_ 2.

- **error** threshold to descriminate between _sinus_ and _noise_
  - _unit:_ thresh lin (pow3) : float
  - _default:_ 0.5
  - _min:_ 0.
  - _max:_ 1.

- **tr** transient gain for resynthesis
  - _unit:_ amp lin (pow2) : float
  - _default:_ 1.
  - _min:_ 0.
  - _max:_ 2.

- **tr.decay** decay time for detected transients
  - _unit:_ time ms : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ 400.

_following are more advanced mode for experimental behaviours:_

- **envpres** on: more subtle
    off: make some time noise sound like glouglou, harsher  
  - _unit:_ 0/1
  - _default:_ 0

- **stereopres** on: more subtle
    off: make some time noise sound like glouglou, harsher
  - _unit:_ 0/1
  - _default:_ 0

- **envmode** on: lpc (more subtle)
    off: trueenv  
  - _unit:_ 0/1
  - _default:_ 0

- **lcporder** number of poles (filter peaks) in the spectral envelope
  - _unit:_ number : int
  - _default:_ 6
  - _min:_ 1
  - _max:_ 48

- **maxfreq** smooth the spectral envelope
  - _unit:_ Hz : float
  - _default:_ 600
  - _min:_ 0.
  - _max:_ none

- **shapeinv**  waveform shape invariant
    on : more subtle
    off : make some time noise sound like glouglou, harsher
  - _unit:_ 0/1
  - _default:_ 0


- **sinmode**  sinusoidal resynthesis method
    1 : additive (smoother)
    0 : vocoder
  - _unit:_ 0/1
  - _default:_ 0

- **gain** gain in dB
  - _unit:_ dB : float
  - _default:_ 0.
  - _min:_ none
  - _max:_ none


---
#### **reverb**

FDN reverb

```
reverb size=30 decay=40 damping=30 drywet=20

```

- **size** size of the room
  - _unit:_ midiunit : float
  - _default:_ 64.
  - _min:_ 0.
  - _max:_ 127.

- **decay** decay of the room
  - _unit:_ midiunit : float
  - _default:_ 64.
  - _min:_ 0.
  - _max:_ 127.

- **damping**  damping of the room
  - _unit:_ midiunit : float
  - _default:_ 64.
  - _min:_ 0.
  - _max:_ 127.

- **diffusion** diffusion of the room
  - _unit:_ midiunit : float
  - _default:_ 64.
  - _min:_ 0.
  - _max:_ 127.

- **width** stereo width of the reverb
  - _unit:_ midiunit : float
  - _default:_ 64.
  - _min:_ 0.
  - _max:_ 127.

- **drywet** crossfade between dry and wet signal
  - _unit:_ % : float
  - _default:_ 50.
  - _min:_ 0.
  - _max:_ 100.

- **gain** gain in dB
  - _unit:_ dB : float
  - _default:_ 0.
  - _min:_ none
  - _max:_ none


---
#### **granul**

live granulation of audio signal.

```
granul density=0 length=20 delay=700 delay-mod-depth=500
```
- **env-type** chance in % of env reverse playing
  - _unit:_ number integer
  - _default:_ 1
  - _min:_ 1
  - _max:_ 9

- **density** density of overlapping grains. below 8. value for rarefaction, upper 8 for densification.
  - _unit:_ factor : float
  - _default:_ 8.
  - _min:_ 0.
  - _max:_ 16.`

- **density-jit** density randomization amount.
  - _unit:_ milliseconds : float
  - _default:_ O.
  - _min:_ 0.
  - _max:_ 4.

- **delay** delay time in ms of grain start. can act as a normal delay.
  - _unit:_ milliseconds : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ 30000.


- **delay-mod-depth** delay modulation amount.
  - _unit:_ milliseconds : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ 30000.


- **delay-mod-freq** frequency of the randomization of the delay time.
  - _unit:_ milliseconds : float
  - _default:_ 1.
  - _min:_ 0.
  - _max:_ 200.


- **freeze** freeze the live sound recording feeding the granulator.
  - _unit:_ 0/1
  - _default:_ 0



- **detune** transposition of the grain in semitones
  - _unit:_ semitones : float
  - _default:_ 0.
  - _min:_ -48.
  - _max:_ 48.


- **detune-jit** amount of randomization of transposition in semitones
  - _unit:_ semitones : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ 48.


- **amp-jit** amount of randomization of the amplitude of the grains in dB
  - _unit:_ dB : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ 48.


- **amp-pow** distribution of the values for the amplitude randomization
  - _unit:_ milliseconds : float
  - _default:_ 20.
  - _min:_ 0.
  - _max:_ 10000.


- **spat-jit** amount of randomization of the stereo/quadri placement of the grain
  - _unit:_ milliseconds : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ 1.


- **rev-snd** chance in % of sound reverse playing
  - _unit:_ % : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ 100.


- **rev-env** chance in % of env reverse playing
  - _unit:_ % : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ 100.


- **drywet** crossfade in % between dry and wet signal
  - _unit:_ % : float
  - _default:_ 100.
  - _min:_ 0.
  - _max:_ 100.

- **gain** gain in dB
  - _unit:_ dB : float
  - _default:_ 0.
  - _min:_ none
  - _max:_ none


---
#### **additive**

additive analysis synthesis of audio signal

```
additive npeak=2 attack=5 decay=5 freqsmoo=5
  hrm=0.2 randdev=20 freqtransp=-24
```

- **freqtransp** transpose all partials by a certain amount of semitones.
  - _unit:_ semitones : float
  - _default:_ 0.
  - _min:_ none
  - _max:_ none

- **freqshift** shift all partials by a certain amount of Hz.
  - _unit:_ Hz : float
  - _default:_ 0.
  - _min:_ none
  - _max:_ none

- **randdev** random shift choosed at each partial creation
  - _unit:_ Hz : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ 100.

- **attack** attack of amplitude enveloppe of each partials
  - _unit:_ time ms : float
  - _default:_ 5.
  - _min:_ 0.
  - _max:_ none

- **decay** decay of amplitude enveloppe of each partials
  - _unit:_ time ms : float
  - _default:_ 5.
  - _min:_ 0.
  - _max:_ none

- **hrm** crossfade between sinus (0.) <-> bl-square (1.) <-> noise (2.)
  - _unit:_ coeff : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ 2.

- **freqsmoo** smooth frequency change in a specific partial. Use it in conjonction with _filterdev_ parameter.
  - _unit:_ time ms : float
  - _default:_ 5.
  - _min:_ 0.
  - _max:_ none

- **npeak** Numbers of sinusoidal components selected for synthesis
  - _unit:_ number : int
  - _default:_ 4
  - _min:_ 0
  - _max:_ 8

- **trigtime** time between succesive analysis frame  
  - _unit:_ time ms : float
  - _default:_ 5
  - _min:_ 0
  - _max:_ none

- **freqmin** Minimum frequency of selected partials
  - _unit:_ Hz : float
  - _default:_ 50
  - _min:_ 1
  - _max:_ 20000

- **freqmax** Maximum frequency of selected partials
  - _unit:_ Hz : float
  - _default:_ 10000
  - _min:_ 1
  - _max:_ 20000

- **filteramp** Minimum amplitude of selected partials
  - _unit:_ amp dB : float
  - _default:_ -90.
  - _min:_ -90.
  - _max:_ 0.


- **filtertilt** spectrum slope of the threshold for amp filtering.
  - _unit:_ slope : float
  - _default:_ 0.
  - _min:_ -3.
  - _max:_ 3.

- **filterdev** maximum amount of frequency deviation (in semitones) for the partial tracking.
  - _unit:_ semitones : float
  - _default:_ 1.
  - _min:_ 0.
  - _max:_ 48.


- **drywet** crossfade between dry and wet signal
  - _unit:_ % : float
  - _default:_ 100.
  - _min:_ 0.
  - _max:_ 100.

- **stereo** amount of (random) stereo distribution of partials
  - _unit:_ coeff : float
  - _default:_ 1.
  - _min:_ 0.
  - _max:_ 1.

- **gain** db gain
  - _unit:_ dB : float
  - _default:_ 0.
  - _min:_ none
  - _max:_ none

---
#### **sfplayer**

  sfplayer max 4 channels.

  ```
  sfplayer name=player1 open=s48_P_P_A_TRAME_4A-2.wav loop=0 speed=1
  ```

- **gain**  gain for the player
  - _unit:_ dB : float
  - _default:_ 0.
  - _min:_ none
  - _max:_ none

- **speed** speed of the player
  - _unit:_ coeff : float
  - _default:_ 1.
  - _min:_ none
  - _max:_ none

_following are messages and can't be modulated :_

- **open**  open a specific soundfile
  - _unit:_ symbol (no SPACE and shouldn't begin with a number)


- **play** play the sound file from the beginning
  - _unit:_ 0/1

- **stop**  stops the sound file
  - _unit:_ anything

- **loop**  loop soundfile
  - _unit:_ 0/1


---




### Modulators

#### **_commontoall_**

- **name**  

```
modtor( name=myfavoritemodtor , ....
```
  - _unit:_ string
  - _descr:_ define yout own name for this module to be able to address it later.
  you can send parameters directly with **send myfavoritemoddor**

---
#### **line**

  ramp a value

  ```
  line(time=2000 min=0.05 max=0.2)
  ```

- **time**  time to reach the value
  - _unit:_ ms : float
  - _default:_ 2000.
  - _min:_ 1.
  - _max:_ none

- **min** output min
  - _unit:_  float
  - _default:_ -1.

- **max**  output max
  - _unit:_  float
  - _default:_ 1.

- **curve** distribution of values
  - _unit:_  float
  - _default:_ 0.
  - _min:_ -3.
  - _max:_ 3.

_following are messages and can't be modulated :_

- **active**  activate _line_
  - _unit:_ 0/1


---
#### **lfo**

  low frequency oscillator for modulation purpose


  ```
  lfo(freq=0.5 min=0.05 max=0.2)
  ```

- **freq** frequency of the LFO
  - _unit:_ Hz : float
  - _default:_ 0.5
  - _min:_ 0.
  - _max:_ none

- **varifreq** amount of randomization of frequency
  - _unit:_ coeff : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ none

- **mode** waveform 1:sine 2:rect 3:sawup 4:sawdown
  - _unit:_ type : integer
  - _default:_ 1
  - _min:_ 1
  - _max:_ 4

- **min**  output min
  - _unit:_  float
  - _default:_ -1.

- **max** output max
  - _unit:_  float
  - _default:_ 1.

- **curve** distribution of values
  - _unit:_  float
  - _default:_ 0.
  - _min:_ -3.
  - _max:_ 3.

_following are messages and can't be modulated :_

- **active**  
  - _unit:_ 0/1
  - _descr:_ activate _lfo_


---
#### **rand**

  random value generator **with no interpolation**

  ```
  rand(freq=1.,min=0,max=5000,curve=1)
  ```

- **freq** frequency of the gen
  - _unit:_ Hz : float
  - _default:_ 0.5
  - _min:_ 0.
  - _max:_ none

- **varifreq** amount of randomization of frequency
  - _unit:_ coeff : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ none

- **walk** how far is the random number from the previous one (random walks)
  - _unit:_ amount : float
  - _default:_ 1
  - _min:_ 0
  - _max:_ 1

- **min**  output min
  - _unit:_  float
  - _default:_ -1.

- **max** output max
  - _unit:_  float
  - _default:_ 1.

- **curve** distribution of values
  - _unit:_  float
  - _default:_ 0.
  - _min:_ -3.
  - _max:_ 3.

_following are messages and can't be modulated :_

- **active**  activate _rand_
  - _unit:_ 0/1

---
#### **randi**

  random value generator **with interpolation**

  ```
  randi(freq=.5,min=0,max=5000,curve=1)
  ```

- **freq** frequency of the gen
  - _unit:_ Hz : float
  - _default:_ 0.5
  - _min:_ 0.
  - _max:_ none

- **varifreq** amount of randomization of frequency
  - _unit:_ coeff : float
  - _default:_ 0.
  - _min:_ 0.
  - _max:_ none


- **walk**  how far is the random number from the previous one (random walks)
  - _unit:_ amount : float
  - _default:_ 1
  - _min:_ 0
  - _max:_ 1

- **min** min output
  - _unit:_  float
  - _default:_ -1.

- **max**  max output
  - _unit:_  float
  - _default:_ 1.

- **curve** distribution of values
  - _unit:_  float
  - _default:_ 0.
  - _min:_ -3.
  - _max:_ 3.

_following are messages and can't be modulated :_

- **active**  activate _randi_
  - _unit:_ 0/1
  - _descr:_


---
#### **midictl**

  midi ctrl in

  ```
  midictl(num=67 min=0.05 max=0.2)
  ```

- **num** MIDI CC number
  - _unit:_ number : integer
  - _default:_ 67
  - _min:_ 0
  - _max:_ 127

- **min** output min
  - _unit:_  float
  - _default:_ -1.

- **max**  output max
  - _unit:_  float
  - _default:_ 1.

- **curve** distribution of values
  - _unit:_  float
  - _default:_ 0.
  - _min:_ -3.
  - _max:_ 3.

_following are messages and can't be modulated :_

- **active**  
  - _unit:_ 0/1
  - _descr:_ activate _midictl_
