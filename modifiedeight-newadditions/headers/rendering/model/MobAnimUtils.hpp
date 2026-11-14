#pragma once
#include <stdint.h>
#include <cmath>
#include <math/Mth.hpp>

static inline uint32_t animHash(uint32_t x) {
	x ^= x >> 16;
	x *= 0x7feb352dU;
	x ^= x >> 15;
	x *= 0x846ca68bU;
	x ^= x >> 16;
	return x;
}

static inline float animHashFloat(uint32_t x) {
	return (float)(animHash(x) & 0x00FFFFFFU) / 16777216.0f;
}

static inline float animSmoothStep(float t) {
	if (t <= 0.0f) return 0.0f;
	if (t >= 1.0f) return 1.0f;
	return t * t * (3.0f - 2.0f * t);
}

static inline float animWindowHold(float t, float windowLen, uint32_t seed, uint32_t chance255, bool signedDir = true) {
	if (windowLen <= 0.001f) return 0.0f;
	float scaled = t / windowLen;
	int32_t wi = (int32_t)floorf(scaled);
	float u = scaled - (float)wi;
	uint32_t h = animHash((uint32_t)wi * 198491317U + seed * 6542989U + 0x9E3779B9U);
	if ((h & 255U) >= chance255) return 0.0f;
	float startU = 0.04f + 0.24f * animHashFloat(h + 1U);
	float durU = 0.36f + 0.50f * animHashFloat(h + 2U);
	if (startU + durU > 0.96f) durU = 0.96f - startU;
	if (u < startU || u > startU + durU) return 0.0f;
	float local = (u - startU) / durU;
	float env = 1.0f;
	if (local < 0.26f) {
		env = animSmoothStep(local / 0.26f);
	} else if (local > 0.70f) {
		env = 1.0f - animSmoothStep((local - 0.70f) / 0.30f);
	}
	float amp = 0.52f + 0.48f * animHashFloat(h + 3U);
	float dir = (!signedDir || ((h >> 8) & 1U)) ? 1.0f : -1.0f;
	return env * amp * dir;
}

static inline float animBurstWave(float t, float windowLen, uint32_t seed, uint32_t chance255, float cycles) {
	if (windowLen <= 0.001f) return 0.0f;
	float scaled = t / windowLen;
	int32_t wi = (int32_t)floorf(scaled);
	float u = scaled - (float)wi;
	uint32_t h = animHash((uint32_t)wi * 374761393U + seed * 668265263U + 0x85EBCA6BU);
	if ((h & 255U) >= chance255) return 0.0f;
	float startU = 0.05f + 0.22f * animHashFloat(h + 5U);
	float durU = 0.34f + 0.48f * animHashFloat(h + 6U);
	if (startU + durU > 0.95f) durU = 0.95f - startU;
	if (u < startU || u > startU + durU) return 0.0f;
	float local = (u - startU) / durU;
	float env = Mth::sin(local * 3.14159265f);
	float freq = cycles * (0.82f + 0.36f * animHashFloat(h + 7U));
	float amp = 0.55f + 0.45f * animHashFloat(h + 8U);
	float dir = ((h >> 9) & 1U) ? 1.0f : -1.0f;
	return env * Mth::sin(local * freq * 6.2831853f) * amp * dir;
}
