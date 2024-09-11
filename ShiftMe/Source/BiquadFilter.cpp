#include "BiquadFilter.hpp"

BiquadFilter::BiquadFilter(int sampleRate) : sampleRate(sampleRate) {}

BiquadFilter::BiquadFilter(struct BiquadFilterCoefficients coeffs,
	int sampleRate)
	: sampleRate(sampleRate), coefficients(coeffs) {}

BiquadFilter::BiquadFilter(enum BiquadFilterType type,
	struct BiquadFilterParams params, int sampleRate)
	: sampleRate(sampleRate) {
	this->setParameters(type, params);
}

void BiquadFilter::setParameters(enum BiquadFilterType filterType,
	struct BiquadFilterParams params) {
	this->type = filterType;
	this->parameters = params;

	this->updateParameters();
}

void BiquadFilter::setSampleRate(int sRate) {
	this->sampleRate = sRate;

	this->updateParameters();
}

void BiquadFilter::updateParameters() {
	if (this->type == UNKOWN) return;

	if (this->parameters.Q == 0) return;

	const float A = (float)std::pow(10, this->parameters.gain / 40),
		sqrtA = std::sqrt(A);
	const float om = 3.141592653589793f * 2 * this->parameters.f / this->sampleRate;
	const float a = std::sin(om) / (2 * this->parameters.Q);
	const float cs = std::cos(om);
	switch (this->type) {
	case ALLPASS:
		this->coefficients = {
			1 - a,
			-2 * cs,
			1 + a,
			 1 + a,
			-2 * cs,
			1 - a
		};
		break;
	case LOWPASS:
		this->coefficients = {
			(1 - cs) / 2,
			1 - cs,
			(1 - cs) / 2,
			1 + a,
			-2 * cs,
			1 - a
		};
		break;
	case HIGHPASS:
		this->coefficients = {
			(1 + cs) / 2,
			-1 - cs,
			(1 + cs) / 2,
			1 + a,
			-2 * cs,
			1 - a
		};
		break;
	case BANDPASS:
		this->coefficients = {
			a,
			0,
			-a,
			1 + a,
			-2 * cs,
			1 - a
		};
		break;
	case NOTCH:
		this->coefficients = {
			1,
			-2 * cs,
			1,
			1 + a,
			-2 * cs,
			1 - a
		};
		break;
	case PEAK:
		this->coefficients = {
			1 + a * A,
			-2 * cs,
			1 - a * A,
			1 + a / A,
			-2 * cs,
			1 - a / A
		};
		break;
	case LOWSHELF:
		this->coefficients = {
			A * ((A + 1) - (A - 1) * cs + 2 * sqrtA * a),
			2 * A * ((A - 1) - (A + 1) * cs),
			A * ((A + 1) - (A - 1) * cs - 2 * sqrtA * a),
			(A + 1) + (A - 1) * cs + 2 * sqrtA * a,
			-2 * ((A - 1) + (A + 1) * cs),
			(A + 1) + (A - 1) * cs - 2 * sqrtA * a
		};
		break;
	case HIGHSHELF:
		this->coefficients = {
			A * ((A + 1) + (A - 1) * cs + 2 * sqrtA * a),
			-2 * A * ((A - 1) + (A + 1) * cs),
			A * ((A + 1) + (A - 1) * cs - 2 * sqrtA * a),
			(A + 1) - (A - 1) * cs + 2 * sqrtA * a,
			2 * ((A - 1) - (A + 1) * cs),
			(A + 1) - (A - 1) * cs - 2 * sqrtA * a
		};
		break;
	default:
		break;
	}
}

void BiquadFilter::setParameters(struct BiquadFilterCoefficients coeffs) {
	this->coefficients = coeffs;
	this->type = UNKOWN;
}

void BiquadFilter::processBlock(float* buffer, int size,
	struct SOState& state) const {
	for (int i = 0; i < size; i++) {
		float yn = coefficients.b0 * buffer[i] + coefficients.b1 * state.x1 +
			coefficients.b2 * state.x2 - coefficients.a1 * state.y1 -
			coefficients.a2 * state.y2;
		yn /= coefficients.a0;
		state.x2 = state.x1;
		state.x1 = buffer[i];
		state.y2 = state.y1;
		state.y1 = yn;
		buffer[i] = yn;
	}
}