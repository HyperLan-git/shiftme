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
                                              -20000, 20000, 0)) {
    this->addParameter(frequency);

    // TODO see Parks-McCellan algo FIR Hilbert
    for (int i = 1; i <= MAX_WINDOW / 2; i += 2) {
        const float x = MAX_WINDOW / 2 - i + 1;
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
        this->hWindow[MAX_WINDOW - i + 1] = -this->hWindow[i];
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

void ShiftMeAudioProcessor::setCurrentProgram(int index) {}

const juce::String ShiftMeAudioProcessor::getProgramName(int index) {
    return {};
}

void ShiftMeAudioProcessor::changeProgramName(int index,
                                              const juce::String& newName) {}

void ShiftMeAudioProcessor::prepareToPlay(double sampleRate,
                                          int samplesPerBlock) {}

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

float theta = pi / 2;
void ShiftMeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    const int inputs = getTotalNumInputChannels();
    const int outputs = getTotalNumOutputChannels();

    if (inputs < 2) return;

    const int samples = buffer.getNumSamples(), rate = getSampleRate();

    constexpr int BLKSIZE = MAX_WINDOW * sizeof(float);

    const float shift = *frequency;
    const float dphi = -shift * tpi / rate;
    theta += dphi * samples;

    float* l = buffer.getWritePointer(0);
    float* r = buffer.getWritePointer(1);

    // The entirety of block is delayed by MAX_WINDOW samples
    std::memmove(block, block + (samples % MAX_WINDOW), BLKSIZE);
    for (int sample = 0; sample < samples; sample += MAX_WINDOW) {
        if (sample + MAX_WINDOW >= samples) {
            std::memmove(block, block + MAX_WINDOW, BLKSIZE);
            std::memcpy(block + MAX_WINDOW, l + sample,
                        (samples % MAX_WINDOW) * sizeof(float));
        } else {
            if (sample != 0) std::memmove(block, block + MAX_WINDOW, BLKSIZE);
            std::memcpy(block + MAX_WINDOW, l + sample, BLKSIZE);
        }

        for (int i = 0; i < MAX_WINDOW && i + sample < samples; i++) {
            std::memcpy(block2, block + i, BLKSIZE);
            juce::FloatVectorOperations::multiply(block2, this->hWindow,
                                                  MAX_WINDOW);
            // Acc is the signal phase shifted by pi/2
            float acc = 0;
            for (int j = 0; j < MAX_WINDOW; j++) {
                acc += block2[j];
            }
            const float I = std::cos(theta + dphi * (sample + i)),
                        Q = std::sin(theta + dphi * (sample + i));

            // DELAY of MAX_WINDOW - MAX_WINDOW / 2 = MAX_WINDOW / 2
            const float del = block[i + MAX_WINDOW / 2];
            l[sample + i] = I * del - acc * Q;
            r[sample + i] = I * del - acc * Q;
        }
    }
}

bool ShiftMeAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ShiftMeAudioProcessor::createEditor() {
    return new ShiftMeAudioProcessorEditor(*this);
}

void ShiftMeAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {}

void ShiftMeAudioProcessor::setStateInformation(const void* data,
                                                int sizeInBytes) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new ShiftMeAudioProcessor();
}
