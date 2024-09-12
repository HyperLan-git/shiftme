#include "PluginProcessor.hpp"

ShiftMeAudioProcessor::ShiftMeAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              )
#endif
      ,
      frequency(new juce::AudioParameterFloat({"value", 1}, "Shift value",
                                              -20000, 20000, 0)),
      antialiasing(new juce::AudioParameterBool({"antialiasing", 1},
                                                "Anti aliasing", false)),
      phaseMode(new juce::AudioParameterBool({"phase mode", 1}, "Phase mode",
                                             false)) {
    this->addParameter(frequency);
    this->addParameter(phaseMode);
    this->addParameter(antialiasing);

    // TODO see Parks-McCellan algo FIR Hilbert
    for (int i = 0; i <= MAX_WINDOW / 2; i += 2) {
        const float x = (MAX_WINDOW - 1) / 2.f - i;
        if (x == 0) {
            this->hWindow[i] = 0;
            continue;
        }
        this->hWindow[i] = -(.42f - .5f * std::cos(tpi * i / (MAX_WINDOW - 1)) +
                             .08f * std::cos(2 * tpi * i / (MAX_WINDOW - 1))) *
                           (.630828f / x);
    }
    for (int i = 0; i <= MAX_WINDOW / 2; i += 2) {
        float v = -this->hWindow[i];
        this->hWindow[MAX_WINDOW - i - 1] = v;
    }

    this->aa.setParameters(HIGHPASS, {0, .707, 0});
}

ShiftMeAudioProcessor::~ShiftMeAudioProcessor() {}

const juce::String ShiftMeAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool ShiftMeAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ShiftMeAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ShiftMeAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double ShiftMeAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int ShiftMeAudioProcessor::getNumPrograms() { return 1; }

int ShiftMeAudioProcessor::getCurrentProgram() { return 0; }

void ShiftMeAudioProcessor::setCurrentProgram(int index) { (void)index; }

const juce::String ShiftMeAudioProcessor::getProgramName(int index) {
    (void)index;
    return {};
}

void ShiftMeAudioProcessor::changeProgramName(int index,
                                              const juce::String& newName) {
    (void)index;
    (void)newName;
}

void ShiftMeAudioProcessor::prepareToPlay(double sampleRate,
                                          int samplesPerBlock) {
    (void)samplesPerBlock;
    this->aa.setSampleRate((int)sampleRate);
}

void ShiftMeAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ShiftMeAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void ShiftMeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer& midiMessages) {
    (void)midiMessages;

    juce::ScopedNoDenormals noDenormals;
    const int inputs = getTotalNumInputChannels();
    const int outputs = getTotalNumOutputChannels();

    if (inputs < 2 || outputs != inputs) return;

    const int samples = buffer.getNumSamples();
    const double rate = getSampleRate();

    // TODO handle small block sizes
    constexpr int BLKSIZE = MAX_WINDOW * sizeof(float);

    const float shift = *frequency;
    const long double dphi = -shift * tpi / rate;

    const bool phase = *phaseMode;

    if (phase)
        theta = shift;
    else
        theta += dphi * samples;

    const float Ic = (float)std::cos(theta), Qc = (float)std::sin(theta);

    float* l = buffer.getWritePointer(0);
    float* r = buffer.getWritePointer(1);

    if (*antialiasing) {
        if (shift < 0)
            aa.setParameters(HIGHPASS, {50 - shift, .707f, 0});
        else
            aa.setParameters(LOWPASS, {22000 - shift, .707, 0});
        aa.processBlock(l, samples, l_s_aa);
        aa.processBlock(r, samples, r_s_aa);
        aa.processBlock(l, samples, l_s2_aa);
        aa.processBlock(r, samples, r_s2_aa);
    }

    // The entirety of this block is delayed by MAX_WINDOW samples

    // First we slide the last samples of the previous pass
    std::memmove(block_l, block_l + (samples % MAX_WINDOW), BLKSIZE);
    std::memmove(block_r, block_r + (samples % MAX_WINDOW), BLKSIZE);
    for (int sample = 0; sample < samples; sample += MAX_WINDOW) {
        if (sample + MAX_WINDOW >= samples) {
            // For the last window
            // We add only the remaining bytes
            std::memmove(block_l, block_l + MAX_WINDOW, BLKSIZE);
            std::memcpy(block_l + MAX_WINDOW, l + sample,
                        (samples % MAX_WINDOW) * sizeof(float));

            std::memmove(block_r, block_r + MAX_WINDOW, BLKSIZE);
            std::memcpy(block_r + MAX_WINDOW, r + sample,
                        (samples % MAX_WINDOW) * sizeof(float));
        } else {
            // For each window we slide by the size of the window and insert
            if (sample > 0) {
                std::memmove(block_l, block_l + MAX_WINDOW, BLKSIZE);
                std::memmove(block_r, block_r + MAX_WINDOW, BLKSIZE);
            }
            std::memcpy(block_l + MAX_WINDOW, l + sample, BLKSIZE);
            std::memcpy(block_r + MAX_WINDOW, r + sample, BLKSIZE);
        }

        for (int i = 0; i < MAX_WINDOW && i + sample < samples; i++) {
            // This is the signal phase shifted by pi/2
            float acc_l = 0;
            float acc_r = 0;
            for (int j = 0; j < MAX_WINDOW; j++) {
                acc_l += block_l[i + j] * this->hWindow[j];
                acc_r += block_r[i + j] * this->hWindow[j];
            }

            // Now we just have to combine both
            if (phase) {
                // DELAY of MAX_WINDOW - MAX_WINDOW / 2 = MAX_WINDOW / 2
                l[sample + i] = Ic * block_l[MAX_WINDOW / 2 + i] - acc_l * Qc;
                r[sample + i] = Ic * block_r[MAX_WINDOW / 2 + i] - acc_r * Qc;
            } else {
                const float I = (float)std::cos(theta + dphi * (sample + i)),
                            Q = (float)std::sin(theta + dphi * (sample + i));

                // DELAY of MAX_WINDOW - MAX_WINDOW / 2 = MAX_WINDOW / 2
                l[sample + i] = I * block_l[MAX_WINDOW / 2 + i] - acc_l * Q;
                r[sample + i] = I * block_l[MAX_WINDOW / 2 + i] - acc_r * Q;
            }
        }
    }
}

bool ShiftMeAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ShiftMeAudioProcessor::createEditor() {
    return new ShiftMeAudioProcessorEditor(*this);
}

void ShiftMeAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    juce::MemoryOutputStream stream(destData, true);
    stream.writeFloat(frequency->convertTo0to1(*frequency));
    stream.writeBool(*antialiasing);
    stream.writeBool(*phaseMode);
}

void ShiftMeAudioProcessor::setStateInformation(const void* data,
                                                int sizeInBytes) {
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes),
                                   false);
    frequency->setValueNotifyingHost(stream.readFloat());
    antialiasing->setValueNotifyingHost(stream.readBool() ? 1 : 0);
    phaseMode->setValueNotifyingHost(stream.readBool() ? 1 : 0);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new ShiftMeAudioProcessor();
}
