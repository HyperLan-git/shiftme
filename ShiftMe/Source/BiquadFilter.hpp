#pragma once

#include <cmath>
#include <string>

enum BiquadFilterType {
    UNKOWN = 0,
    ALLPASS = 1,
    LOWPASS = 2,
    HIGHPASS = 3,
    BANDPASS = 4,
    NOTCH = 5,
    PEAK = 6,
    LOWSHELF = 7,
    HIGHSHELF = 8
};

struct BiquadFilterParams {
    float f;
    float Q;
    float gain;
};

struct SOState {
    float x1, x2, y1, y2;
};

struct BiquadFilterCoefficients {
    bool operator==(BiquadFilterCoefficients other) const {
        return this->a0 == other.a0 && this->a1 == other.a1 &&
               this->a2 == other.a2 && this->b0 == other.b0 &&
               this->b1 == other.b1 && this->b2 == other.b2;
    }

    inline bool operator!=(BiquadFilterCoefficients other) const {
        return !this->operator==(other);
    }

    float b0 = 1;
    float b1 = 0;
    float b2 = 0;
    float a0 = 1;
    float a1 = 0;
    float a2 = 0;
};

class BiquadFilter {
   public:
    BiquadFilter(int sampleRate = 44100);
    BiquadFilter(struct BiquadFilterCoefficients coeffs,
                 int sampleRate = 44100);
    BiquadFilter(enum BiquadFilterType type, struct BiquadFilterParams params,
                 int sampleRate = 44100);

    void setParameters(enum BiquadFilterType type,
                       struct BiquadFilterParams params);
    void setParameters(struct BiquadFilterCoefficients coeffs);
    void setSampleRate(int sampleRate);

    inline int getSampleRate() const { return this->sampleRate; }
    inline enum BiquadFilterType getType() const { return this->type; }
    inline struct BiquadFilterParams getParameters() const {
        if (this->type == UNKOWN) return {};
        return this->parameters;
    }
    inline struct BiquadFilterCoefficients getCoeffs() const {
        return this->coefficients;
    }

    // The state struct should be conserved between blocks of the same channel
    void processBlock(float* buffer, int size, struct SOState& state) const;

   private:
    void updateParameters();

    struct BiquadFilterCoefficients coefficients = {};

    int sampleRate = 44100;
    enum BiquadFilterType type = UNKOWN;
    struct BiquadFilterParams parameters = {};
};