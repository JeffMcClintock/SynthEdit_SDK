#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include "Sinc.h"
#include "../shared/xp_simd.h"
#include "./SincFilter.h"

REGISTER_PLUGIN2 (SincFilterLpHp, L"SE Sinc Lowpass Filter" );

SincFilterLpHp::SincFilterLpHp( )
{
	// Register pins.
	initializePin(pinSignal);
	initializePin(pinCuttoffkHz);
	initializePin(pinTaps);
	initializePin(pinOutput);
}

void SincFilterLpHp::subProcess(int sampleFrames)
{
	// get pointers to in/output buffers.
	const float* signal = getBuffer(pinSignal);
	float* output = getBuffer(pinOutput);

	const auto numCoefs = coefs.size();

	// maintain history as contiguous samples by shifting left each time.
	memcpy(&(hist[numCoefs]), signal, sizeof(float) * sampleFrames);

#ifndef GMPI_SSE_AVAILABLE

	// Process first coefs (copy).
	float* h = hist.data();
	auto out = output;
	for (int s = 0; s < sampleFrames; ++s)
	{
		*out++ = h[s] * coefs[0];
	}
	++h;

	// Process remaining coefs (add).
	for (int t = 1; t < numCoefs; ++t)
	{
		out = output;
		for (int s = 0; s < sampleFrames; ++s)
		{
			*out++ += h[s] * coefs[t];
		}
		++h;
	}

#else
	// Process 4 samples at a time.

	float* h = hist.data();

	// Process first coefs (copy).
	float* h2 = h++;
	__m128 tap = _mm_set_ps1(coefs[0]);
	float* out = output;
	for (int s = 0; s < sampleFrames; s += 4)
	{
		_mm_storeu_ps(out, _mm_mul_ps(_mm_load_ps(h2), tap));
		h2 += 4;
		out += 4;
	}

	// Process remaining coefs (add).
	for (int t = 1; t < numCoefs; ++t)
	{
		h2 = h++;
		tap = _mm_set_ps1(coefs[t]);
		out = output;
		for (int s = 0; s < sampleFrames; s += 4)
		{
			// output[s] += h[s] * taps[t];
			_mm_storeu_ps(out, _mm_add_ps(_mm_loadu_ps(out), _mm_mul_ps(_mm_loadu_ps(h2), tap)));
			h2 += 4;
			out += 4;
		}
	}

#endif

	// shift history.
	memcpy(hist.data(), &hist[sampleFrames], sizeof(float) * numCoefs);
}

void SincFilterLpHp::subProcessStatic(int sampleFrames)
{
	subProcess(sampleFrames);

	if (staticCount < sampleFrames && pinOutput.isStreaming())
	{
		assert(staticCount >= 0);
		pinOutput.setStreaming(false, getBlockPosition() + staticCount);
	}

	staticCount -= sampleFrames;
}

void SincFilterLpHp::onSetPins()
{
	if (pinTaps.isUpdated() || pinCuttoffkHz.isUpdated() )
	{
		double cuttoff = 1000.0f * pinCuttoffkHz / getSampleRate();
		const int sseAlign = 4;
		int alignedTaps = ((std::max)(1, pinTaps.getValue()) + sseAlign - 1) & -(sseAlign);
		coefs.resize(alignedTaps);
		calcWindowedSinc(cuttoff, alignedTaps, coefs.data());

		int32_t blockSize;
		getHost()->getBlockSize(blockSize);
		const int numCoefs = static_cast<int>(coefs.size());
		hist.assign(numCoefs + blockSize, 0.0f);
	}

	// Set state of output audio pins.
	pinOutput.setStreaming(true);

	// Set processing method.
	if (pinSignal.isStreaming())
	{
		setSubProcess(&SincFilterLpHp::subProcess);
	}
	else
	{
		staticCount = static_cast<int>(hist.size());
		setSubProcess(&SincFilterLpHp::subProcessStatic);
	}

	// Set sleep mode (optional).
	// setSleep(false);
}


/*
float* h = hist;
// convolution.
for (int s = sampleFrames; s > 0; --s)
{
float sum = 0;
for (int t = 0; t < numCoefs; ++t)
{
sum += h[t] * taps[t];
}
*output++ = sum;
++h;
}
*/
/*
// version without memcpy, slower.

// write new values into history buffer.
int todo = (std::min)(sampleFrames, histSize - histIdx);
int s;
for (s = 0; s < todo; ++s)
{
hist[histIdx++] = signal[s];
}
if (histIdx == histSize)
{
histIdx = 0;
for (; s < sampleFrames; ++s)
{
hist[histIdx++] = signal[s];
}
}

// zero output buffer.
memset(output, 0, sizeof(float) * sampleFrames);

// convolution.
int r = histReadIdx;
for (int t = 0; t < numCoefs; ++t)
{
int q = r;
int todo = (std::min)(sampleFrames, histSize - q);
int s;
for (s = 0; s < todo; ++s)
{
output[s] += hist[q++] * taps[t];
}

if (q == histSize)
{
q = 0;
for (; s < sampleFrames; ++s)
{
output[s] += hist[q++] * taps[t];
}
}

if (++r >= histSize)
r = 0;
}

histReadIdx += sampleFrames;
if (histReadIdx >= histSize)
histReadIdx -= histSize;

#endif
#endif

#else
for (int s = sampleFrames; s > 0; --s)
{
hist[histIdx++] = *signal++;
if (histIdx == numCoefs)
{
histIdx = 0;
}

// convolution.
float sum = 0;
int x = histIdx;
for (int t = 0; t < numCoefs; ++t)
{
sum += hist[x++] * taps[t];
if (x == numCoefs)
{
x = 0;
}
}
*output++ = sum;
}
#endif
*/

/*
// Hand-made memcpy, no faster.
__m128* dest = (__m128*) hist;
float* src = &hist[sampleFrames];
for (int s = 0; s < histSize - sampleFrames + 3; s = s + 4)
{
*dest++ = _mm_loadu_ps(src);
src += 4;
}
*/
/*




int _tmain(int argc, _TCHAR* argv[])
{
const int sincTapCount = 171;

float sincTaps[sincTapCount];

calcWin dowedSinc( 0.25, sincTapCount, sincTaps );
/*
for( int i = 0 ; i < sincTapCount ; ++i )
{
_RPT1( _CRT_WARN, "%f\n", sincTaps[i] );
}
* /
int bufferIndex = 0; // whatever
float samples[2000];
for( int i = 0 ; i < 2000 ; ++i )
{
samples[i] = ( i & 0x20 ) == 0 ? 1.0f : -1.0f;
}

float buffer[sincTapCount];
memset( buffer, 0 , sizeof(buffer) );
int sampleFrames = 200;


float* in = samples;

for( int s = 0 ; s < sampleFrames ; ++s )
{
// store input history to bufffer.
buffer[bufferIndex++] = *in++;
if( bufferIndex == sincTapCount )
{
bufferIndex = 0;
}

// convolve SINC FIR with bufer.
float a = 0;
int bi = bufferIndex;
for( int i = 0 ; i < sincTapCount ; ++i )
{
a += sincTaps[i] * buffer[bufferIndex++];
if( bufferIndex == sincTapCount )
{
bufferIndex = 0;
}
}

// *out++ - a;
_RPT2( _CRT_WARN, "%d, %f\n", s, a );
}

return 0;
}

*/
