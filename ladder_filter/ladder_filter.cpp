#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

// Declare a local daisy_petal for hardware access
DaisyPetal hw;
MoogLadder flt;
Oscillator lfo;

bool      bypass;

Led led1, led2;

Parameter lfo_speed, amplitude, res_freq;

// This runs at a fixed rate, to prepare audio samples
void callback(float *in, float *out, size_t size)
{
    hw.ProcessAllControls();
    led1.Update();
    led2.Update();
    lfo.SetFreq(lfo_speed.Process());
    lfo.SetAmp(amplitude.Process());
    flt.SetRes(res_freq.Process());

    // (De-)Activate bypass and toggle LED when left footswitch is pressed
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
        led1.Set(bypass ? 0.0f : 1.0f);
    }

    for(size_t i = 0; i < size; i += 2)
    {
        float dryl, dryr, freq, outputl, outputr;
        dryl  = in[i];
        dryr  = in[i + 1];

        freq = 5000 + (lfo.Process() * 5000);

        flt.SetFreq(freq);
        outputl = flt.Process(dryl);
        outputr = flt.Process(dryr);

        if(bypass)
        {
            out[i]     = in[i];     // left
            out[i + 1] = in[i + 1]; // right
        }
        else
        {
            out[i] = outputl;
            out[i + 1] = outputr;
        }
    }
}

int main(void)
{
    float samplerate;

    hw.Init();
    samplerate = hw.AudioSampleRate();

    // initialize Moogladder object
    flt.Init(samplerate);
    flt.SetRes(0.7);

    // set parameters for sine oscillator object
    lfo.Init(samplerate);
    lfo.SetWaveform(Oscillator::WAVE_SIN);
    lfo.SetAmp(0.5);
    lfo.SetFreq(.4);

    // Initialize your knobs here like so:
    lfo_speed.Init(hw.knob[Terrarium::KNOB_1], 0.1f, 0.999f, Parameter::LINEAR);
    amplitude.Init(hw.knob[Terrarium::KNOB_2], 0.5f, 0.999f, Parameter::LINEAR);
    res_freq.Init(hw.knob[Terrarium::KNOB_3], 0.1f, 0.8f, Parameter::LOGARITHMIC);

    // Set samplerate for your processing like so:
    // verb.Init(samplerate);


    // Init the LEDs and set activate bypass
    led1.Init(hw.seed.GetPin(Terrarium::LED_1),false);
    led1.Update();
    bypass = true;

    hw.StartAdc();
    hw.StartAudio(callback);
    while(1)
    {
        // Do Stuff Infinitely Here
        System::Delay(10);
    }
}