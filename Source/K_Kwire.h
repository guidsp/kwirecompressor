#pragma once
#include <JuceHeader.h>
using namespace juce;

template<int chNum>
class K_Kwire{
public:
	K_Kwire() {
	}

	void setupParams(float initRatio, float initThreshold, float initAttack, float initRelease, double initSampleRate){
		sampleRate = initSampleRate;
		ratio = initRatio;
		threshold = initThreshold;
		attackInSamps = initAttack * sampleRate * 0.001;
		releaseInSamps = initRelease * sampleRate * 0.001;
		driveTime = driveTimeInMS * sampleRate * 0.001;
	}

	inline void updateParams(float inRatio, float inThreshold, float inAttack, float inRelease) {
		ratio = inRatio;
		threshold = inThreshold;
		attackInSamps = inAttack * sampleRate * 0.001;
		releaseInSamps = inRelease * sampleRate * 0.001;
	}
	
	inline void overdrive(dsp::AudioBlock<float>& block){
		for (int channel = 0; channel < chNum; ++channel) {

			channelData[channel] = block.getChannelPointer(channel);

			for (int sample = 0; sample < block.getNumSamples(); ++sample){

				float &input = channelData[channel][sample];

				if (input >= 0.0) { //For positive signal values
					//get preliminar envelope
					driveEnv[channel] = slide(abs(input), prevDriveEnv[channel], driveTime * 0.015f);
					prevDriveEnv[channel] = driveEnv[channel];

					//is not in clipping territory
					bool clip = (driveEnv[channel] < 0.5f);

					//get envelope
					drive[channel] = slide(abs(0.5f * input), prevDrive[channel], driveTime - 0.99 * driveTime * clip, 1.1);

					drive[channel] = jlimit(0.f, 1.f, drive[channel]);

					//set prev envelope 
					prevDrive[channel] = drive[channel];

					drive[channel] = (drive[channel] * (2.0 - drive[channel])); //logarithmic distribution

					float dry = input; //for dry/wet mix controlled by ratio

					//Sigmoid
					input = input * ((27.0f + ((9.0f - 8.2f * drive[channel]) * input * input * 0.8f)) / (27.0f + 9.0f * input * input));
					input = input * ((27.0f + 0.8f * input * input) / (27.0f + 9.0f * input * input));

					input = input * 0.9f;
					input = input * (float)(input < 0.647f) + 0.9 * (input - 0.1841) * (2.2 - input) * (float)((input > 0.647f) && (input < 1.192f)) + 0.9144 * (float)(input >= 1.192f);
					input = input * 1.1111111f;

					float &wet = input;

					float dryAmt = (2.f - ratio);
					float wetAmt = (ratio - 1.f); //amount of OD according to ratio

					channelData[channel][sample] = dry * dryAmt + wet * wetAmt;

				} else {
					float dry = input; //for dry/wet mix controlled by ratio

					input = -input;

					//Sigmoid
					input = -1.f * (input * (float)(input < 0.647f) + 0.9 * (input - 0.1841) * (2.2 - input) * (float)((input > 0.647f) && (input < 1.192f)) + 0.9144 * (float)(input >= 1.192f));

					float* wet = &input;

					float dryAmt = (2.f - ratio);
					float wetAmt = (ratio - 1.f); //amount of OD according to ratio

					channelData[channel][sample] = dry * dryAmt + *wet * wetAmt;
				}
			}
		}
	}
	
	inline void compress(dsp::AudioBlock<float>& block) {
		for (int channel = 0; channel < chNum; ++channel)
			channelData[channel] = block.getChannelPointer(channel);

		for (int channel = 0; channel < chNum; ++channel) {
			for (int sample = 0; sample < block.getNumSamples(); ++sample)
			{
				//attenuation calculation
				rawAttenuation[channel] = calcAttenuation(ratio, threshold, Decibels::gainToDecibels(abs(channelData[channel][sample])), compKnee);

				//envelope follower
				if (rawAttenuation[channel] > prevEnvelope[channel]) //release
					envelope[channel] = slide(rawAttenuation[channel], prevEnvelope[channel], releaseInSamps, 1.1f);
				else //attack
					envelope[channel] = slide(rawAttenuation[channel], prevEnvelope[channel], attackInSamps, 1.1f);

				prevEnvelope[channel] = envelope[channel];

				channelData[channel][sample] *= envelope[channel];
			}
		}
	}

private:
	//No knee
	inline float calcAttenuation(const float &ratio, const float &threshold, const float &signalInDB) {

		return 1.f + (signalInDB > threshold) * -(1.f - Decibels::decibelsToGain((threshold - signalInDB) * (1.f - (1.f / ratio))));
	}

	//Soft knee
	inline float calcAttenuation(const float& ratio, const float& threshold, const float& signalInDB, const float &kneeInDbs) {

		//auto knee = 1.f - (jlimit(0.f, kneeInDbs, threshold - signalInDB) / kneeInDbs);

		return 1.f + (1.f - (jlimit(0.f, kneeInDbs, threshold - signalInDB) / kneeInDbs)) *
			-(1.f - Decibels::decibelsToGain((threshold - signalInDB) * (1.f - (1.f / ratio))));
	}

	inline float slide(const float &input, const float &prevOutput, const float &steps) {
		//y (n) = y (n-1) + (x (n) - y (n-1))/steps
		return prevOutput + (input - prevOutput) / steps;
	}

	inline float slide(const float &input, const float &prevOutput, const float &steps, const float &speedFactor)
	{
		//y (n) = y (n-1) + (x (n) - y (n-1))/steps
		return prevOutput + (input - prevOutput) / (steps * speedFactor);
	}

	constexpr static float driveTimeInMS = 1100.f;
	constexpr static float compKnee = 5.f;

	double sampleRate;

	float* channelData[chNum];

	float ratio,
		threshold,
		attackInSamps,
		releaseInSamps,
		driveTime;

	float envelope[chNum],
		prevEnvelope[chNum] = { 0.f },
		rawAttenuation[chNum],
		drive[chNum],
		prevDrive[chNum] = { 0.f },
		driveEnv[chNum],
		prevDriveEnv[chNum] = { 0.f };
};