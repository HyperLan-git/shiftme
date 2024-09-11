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
      frequency(new juce::AudioParameterFloat({"frequency", 1}, "Frequency",
                                              -20000, 20000, 0)),
      antialiasing(new juce::AudioParameterBool({"antialiasing", 1}, "Anti aliasing", false)) {
    this->addParameter(frequency);
    this->addParameter(antialiasing);

    // TODO see Parks-McCellan algo FIR Hilbert
    for (int i = 1; i <= MAX_WINDOW / 2; i += 2) {
        const float x = MAX_WINDOW / 2.f - i + 1;
        if (x == 0) {
            this->hWindow[i] = 0;
            continue;
        }
        float sn = std::sin(.5f * pi * x);
        this->hWindow[i] =
            -(0.54f - 0.46f * std::cos(2 * pi * i / (MAX_WINDOW - 1))) *
            (sn * sn / x);
    }
    for (int i = 1; i <= MAX_WINDOW / 2; i++) {
        float v = -this->hWindow[i];
        this->hWindow[MAX_WINDOW - i + 1] = v;
    }

    float acc = 0;
    for (int i = 0; i < MAX_WINDOW; i++) {
        acc += this->hWindow[i];
    }
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

void ShiftMeAudioProcessor::setCurrentProgram(int index) {
    (void)index;
}

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
    (void)sampleRate;
    (void)samplesPerBlock;
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

    //TODO handle small block sizes
    constexpr int BLKSIZE = MAX_WINDOW * sizeof(float);

    const float shift = *frequency;
    const double dphi = -shift * tpi / rate;
    theta += dphi * samples;

    float* l = buffer.getWritePointer(0);
    float* r = buffer.getWritePointer(1);

    // The entirety of this block is delayed by MAX_WINDOW samples
    std::memmove(block_l, block_l + (samples % MAX_WINDOW), BLKSIZE);
    std::memmove(block_r, block_r + (samples % MAX_WINDOW), BLKSIZE);
    for (int sample = 0; sample < samples; sample += MAX_WINDOW) {
        if (sample + MAX_WINDOW >= samples) {
            std::memmove(block_l, block_l + MAX_WINDOW, BLKSIZE);
            std::memcpy(block_l + MAX_WINDOW, l + sample,
                        (samples % MAX_WINDOW) * sizeof(float));

            std::memmove(block_r, block_r + MAX_WINDOW, BLKSIZE);
            std::memcpy(block_r + MAX_WINDOW, r + sample,
                        (samples % MAX_WINDOW) * sizeof(float));
        } else {
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
                acc_l += block_l[i + j / 2 + MAX_WINDOW / 4] * this->hWindow[j];
                acc_r += block_r[i + j / 2 + MAX_WINDOW / 4] * this->hWindow[j];
            }

            // Now we just have to combine both
            const float I = (float)std::cos(theta + dphi * (sample + i)),
                        Q = (float)std::sin(theta + dphi * (sample + i));

            // DELAY of MAX_WINDOW - MAX_WINDOW / 2 = MAX_WINDOW / 2
            l[sample + i] = I * block_l[i + MAX_WINDOW / 2] - acc_l * Q;
            r[sample + i] = I * block_r[i + MAX_WINDOW / 2] - acc_r * Q;
        }
        //TODO WIP
        if (*antialiasing) {
            std::memset(block_l_aa, 0, sizeof(float) * 4 * MAX_WINDOW);
            std::memset(block_r_aa, 0, sizeof(float) * 4 * MAX_WINDOW);
            for (int i = 0; i < MAX_WINDOW && i + sample < samples; i++) {
                block_l_aa[i * 2] = l[sample + i];
                block_r_aa[i * 2] = r[sample + i];
            }
            //l_aa.processBlock()
        }
    }
}

bool ShiftMeAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ShiftMeAudioProcessor::createEditor() {
    return new ShiftMeAudioProcessorEditor(*this);
}

void ShiftMeAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    (void)destData;
}

void ShiftMeAudioProcessor::setStateInformation(const void* data,
                                                int sizeInBytes) {
    (void)data;
    (void)sizeInBytes;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new ShiftMeAudioProcessor();
}
