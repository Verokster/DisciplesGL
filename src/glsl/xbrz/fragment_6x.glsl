/*
	xBRZ fragment shader
	based on libretro xBRZ shader
	https://github.com/libretro/glsl-shaders/tree/master/xbrz/shaders

	MIT License

	Copyright (c) 2019 Oleksiy Ryabchun

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

precision mediump float;

uniform sampler2D tex01;
uniform sampler2D tex02;
uniform vec2 texSize;

in vec2 fTex;
out vec4 fragColor;

#define BLEND_NONE 0
#define BLEND_NORMAL 1
#define BLEND_DOMINANT 2
#define LUMINANCE_WEIGHT 1.0
#define EQUAL_COLOR_TOLERANCE 30.0/255.0
#define STEEP_DIRECTION_THRESHOLD 2.2
#define DOMINANT_DIRECTION_THRESHOLD 3.6

const float  one_sixth = 1.0 / 6.0;
const float  two_sixth = 2.0 / 6.0;
const float four_sixth = 4.0 / 6.0;
const float five_sixth = 5.0 / 6.0;

float reduce(const vec3 color) {
	return dot(color, vec3(65536.0, 256.0, 1.0));
}

float DistYCbCr(const vec3 pixA, const vec3 pixB) {
	const vec3 w = vec3(0.2627, 0.6780, 0.0593);
	const float scaleB = 0.5 / (1.0 - w.b);
	const float scaleR = 0.5 / (1.0 - w.r);
	vec3 diff = pixA - pixB;
	float Y = dot(diff, w);
	float Cb = scaleB * (diff.b - Y);
	float Cr = scaleR * (diff.r - Y);
	
	return sqrt( ((LUMINANCE_WEIGHT * Y) * (LUMINANCE_WEIGHT * Y)) + (Cb * Cb) + (Cr * Cr) );
}

bool IsPixEqual(const vec3 pixA, const vec3 pixB) {
	return (DistYCbCr(pixA, pixB) < EQUAL_COLOR_TOLERANCE);
}

bool IsBlendingNeeded(const ivec4 blend) {
	return any(notEqual(blend, ivec4(BLEND_NONE)));
}

void main() {
	vec4 check = texture(tex01, fTex);
	if (check == texture(tex02, fTex))
		discard;

	if (check.a != 0.0)
	{
		vec2 texel = floor(fTex * texSize) + 0.5;

		#define TEX(x, y) texture(tex01, (texel + vec2(x, y)) / texSize).rgb

		vec3 src[25];
		src[21] = TEX(-1.0, -2.0);
		src[22] = TEX( 0.0, -2.0);
		src[23] = TEX( 1.0, -2.0);
		src[ 6] = TEX(-1.0, -1.0);
		src[ 7] = TEX( 0.0, -1.0);
		src[ 8] = TEX( 1.0, -1.0);
		src[ 5] = TEX(-1.0,  0.0);
		src[ 0] = TEX( 0.0,  0.0);
		src[ 1] = TEX( 1.0,  0.0);
		src[ 4] = TEX(-1.0,  1.0);
		src[ 3] = TEX( 0.0,  1.0);
		src[ 2] = TEX( 1.0,  1.0);
		src[15] = TEX(-1.0,  2.0);
		src[14] = TEX( 0.0,  2.0);
		src[13] = TEX( 1.0,  2.0);
		src[19] = TEX(-2.0, -1.0);
		src[18] = TEX(-2.0,  0.0);
		src[17] = TEX(-2.0,  1.0);
		src[ 9] = TEX( 2.0, -1.0);
		src[10] = TEX( 2.0,  0.0);
		src[11] = TEX( 2.0,  1.0);
	
		float v[9];
		v[0] = reduce(src[0]);
		v[1] = reduce(src[1]);
		v[2] = reduce(src[2]);
		v[3] = reduce(src[3]);
		v[4] = reduce(src[4]);
		v[5] = reduce(src[5]);
		v[6] = reduce(src[6]);
		v[7] = reduce(src[7]);
		v[8] = reduce(src[8]);
	
		ivec4 blendResult = ivec4(BLEND_NONE);
		if (!((v[0] == v[1] && v[3] == v[2]) || (v[0] == v[3] && v[1] == v[2])))
		{
			float dist_03_01 = DistYCbCr(src[ 4], src[ 0]) + DistYCbCr(src[ 0], src[ 8]) + DistYCbCr(src[14], src[ 2]) + DistYCbCr(src[ 2], src[10]) + (4.0 * DistYCbCr(src[ 3], src[ 1]));
			float dist_00_02 = DistYCbCr(src[ 5], src[ 3]) + DistYCbCr(src[ 3], src[13]) + DistYCbCr(src[ 7], src[ 1]) + DistYCbCr(src[ 1], src[11]) + (4.0 * DistYCbCr(src[ 0], src[ 2]));
			bool dominantGradient = (DOMINANT_DIRECTION_THRESHOLD * dist_03_01) < dist_00_02;
			blendResult.z = ((dist_03_01 < dist_00_02) && (v[0] != v[1]) && (v[0] != v[3])) ? ((dominantGradient) ? BLEND_DOMINANT : BLEND_NORMAL) : BLEND_NONE;
		}

		if (!((v[5] == v[0] && v[4] == v[3]) || (v[5] == v[4] && v[0] == v[3])))
		{
			float dist_04_00 = DistYCbCr(src[17], src[ 5]) + DistYCbCr(src[ 5], src[ 7]) + DistYCbCr(src[15], src[ 3]) + DistYCbCr(src[ 3], src[ 1]) + (4.0 * DistYCbCr(src[ 4], src[ 0]));
			float dist_05_03 = DistYCbCr(src[18], src[ 4]) + DistYCbCr(src[ 4], src[14]) + DistYCbCr(src[ 6], src[ 0]) + DistYCbCr(src[ 0], src[ 2]) + (4.0 * DistYCbCr(src[ 5], src[ 3]));
			bool dominantGradient = (DOMINANT_DIRECTION_THRESHOLD * dist_05_03) < dist_04_00;
			blendResult.w = ((dist_04_00 > dist_05_03) && (v[0] != v[5]) && (v[0] != v[3])) ? ((dominantGradient) ? BLEND_DOMINANT : BLEND_NORMAL) : BLEND_NONE;
		}
	
		if (!((v[7] == v[8] && v[0] == v[1]) || (v[7] == v[0] && v[8] == v[1])))
		{
			float dist_00_08 = DistYCbCr(src[ 5], src[ 7]) + DistYCbCr(src[ 7], src[23]) + DistYCbCr(src[ 3], src[ 1]) + DistYCbCr(src[ 1], src[ 9]) + (4.0 * DistYCbCr(src[ 0], src[ 8]));
			float dist_07_01 = DistYCbCr(src[ 6], src[ 0]) + DistYCbCr(src[ 0], src[ 2]) + DistYCbCr(src[22], src[ 8]) + DistYCbCr(src[ 8], src[10]) + (4.0 * DistYCbCr(src[ 7], src[ 1]));
			bool dominantGradient = (DOMINANT_DIRECTION_THRESHOLD * dist_07_01) < dist_00_08;
			blendResult.y = ((dist_00_08 > dist_07_01) && (v[0] != v[7]) && (v[0] != v[1])) ? ((dominantGradient) ? BLEND_DOMINANT : BLEND_NORMAL) : BLEND_NONE;
		}
	
		if (!((v[6] == v[7] && v[5] == v[0]) || (v[6] == v[5] && v[7] == v[0])))
		{
			float dist_05_07 = DistYCbCr(src[18], src[ 6]) + DistYCbCr(src[ 6], src[22]) + DistYCbCr(src[ 4], src[ 0]) + DistYCbCr(src[ 0], src[ 8]) + (4.0 * DistYCbCr(src[ 5], src[ 7]));
			float dist_06_00 = DistYCbCr(src[19], src[ 5]) + DistYCbCr(src[ 5], src[ 3]) + DistYCbCr(src[21], src[ 7]) + DistYCbCr(src[ 7], src[ 1]) + (4.0 * DistYCbCr(src[ 6], src[ 0]));
			bool dominantGradient = (DOMINANT_DIRECTION_THRESHOLD * dist_05_07) < dist_06_00;
			blendResult.x = ((dist_05_07 < dist_06_00) && (v[0] != v[5]) && (v[0] != v[7])) ? ((dominantGradient) ? BLEND_DOMINANT : BLEND_NORMAL) : BLEND_NONE;
		}
	
		vec3 dst[36];
		dst[ 0] = src[0];
		dst[ 1] = src[0];
		dst[ 2] = src[0];
		dst[ 3] = src[0];
		dst[ 4] = src[0];
		dst[ 5] = src[0];
		dst[ 6] = src[0];
		dst[ 7] = src[0];
		dst[ 8] = src[0];
		dst[ 9] = src[0];
		dst[10] = src[0];
		dst[11] = src[0];
		dst[12] = src[0];
		dst[13] = src[0];
		dst[14] = src[0];
		dst[15] = src[0];
		dst[16] = src[0];
		dst[17] = src[0];
		dst[18] = src[0];
		dst[19] = src[0];
		dst[20] = src[0];
		dst[21] = src[0];
		dst[22] = src[0];
		dst[23] = src[0];
		dst[24] = src[0];
		dst[25] = src[0];
		dst[26] = src[0];
		dst[27] = src[0];
		dst[28] = src[0];
		dst[29] = src[0];
		dst[30] = src[0];
		dst[31] = src[0];
		dst[32] = src[0];
		dst[33] = src[0];
		dst[34] = src[0];
		dst[35] = src[0];
	
		if (IsBlendingNeeded(blendResult) == true)
		{
			float dist_01_04 = DistYCbCr(src[1], src[4]);
			float dist_03_08 = DistYCbCr(src[3], src[8]);
			bool haveShallowLine = (STEEP_DIRECTION_THRESHOLD * dist_01_04 <= dist_03_08) && (v[0] != v[4]) && (v[5] != v[4]);
			bool haveSteepLine   = (STEEP_DIRECTION_THRESHOLD * dist_03_08 <= dist_01_04) && (v[0] != v[8]) && (v[7] != v[8]);
			bool needBlend = (blendResult.z != BLEND_NONE);
			bool doLineBlend = (  blendResult.z >= BLEND_DOMINANT ||
							   ((blendResult.y != BLEND_NONE && !IsPixEqual(src[0], src[4])) ||
								 (blendResult.w != BLEND_NONE && !IsPixEqual(src[0], src[8])) ||
								 (IsPixEqual(src[4], src[3]) && IsPixEqual(src[3], src[2]) && IsPixEqual(src[2], src[1]) && IsPixEqual(src[1], src[8]) && IsPixEqual(src[0], src[2]) == false) ) == false );
		
			vec3 blendPix = ( DistYCbCr(src[0], src[1]) <= DistYCbCr(src[0], src[3]) ) ? src[1] : src[3];
			dst[10] = mix(dst[10], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.250 : 0.000);
			dst[11] = mix(dst[11], blendPix, (needBlend && doLineBlend) ? ((haveSteepLine) ? 0.750 : ((haveShallowLine) ? 0.250 : 0.000)) : 0.000);
			dst[12] = mix(dst[12], blendPix, (needBlend && doLineBlend) ? ((!haveShallowLine && !haveSteepLine) ? 0.500 : 1.000) : 0.000);
			dst[13] = mix(dst[13], blendPix, (needBlend && doLineBlend) ? ((haveShallowLine) ? 0.750 : ((haveSteepLine) ? 0.250 : 0.000)) : 0.000);
			dst[14] = mix(dst[14], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.250 : 0.000);
			dst[25] = mix(dst[25], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.250 : 0.000);
			dst[26] = mix(dst[26], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.750 : 0.000);
			dst[27] = mix(dst[27], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 1.000 : 0.000);
			dst[28] = mix(dst[28], blendPix, (needBlend) ? ((doLineBlend) ? ((haveSteepLine) ? 1.000 : ((haveShallowLine) ? 0.750 : 0.500)) : 0.05652034508) : 0.000);
			dst[29] = mix(dst[29], blendPix, (needBlend) ? ((doLineBlend) ? 1.000 : 0.4236372243) : 0.000);
			dst[30] = mix(dst[30], blendPix, (needBlend) ? ((doLineBlend) ? 1.000 : 0.9711013910) : 0.000);
			dst[31] = mix(dst[31], blendPix, (needBlend) ? ((doLineBlend) ? 1.000 : 0.4236372243) : 0.000);
			dst[32] = mix(dst[32], blendPix, (needBlend) ? ((doLineBlend) ? ((haveShallowLine) ? 1.000 : ((haveSteepLine) ? 0.750 : 0.500)) : 0.05652034508) : 0.000);
			dst[33] = mix(dst[33], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 1.000 : 0.000);
			dst[34] = mix(dst[34], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.750 : 0.000);
			dst[35] = mix(dst[35], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.250 : 0.000);
		
			dist_01_04 = DistYCbCr(src[7], src[2]);
			dist_03_08 = DistYCbCr(src[1], src[6]);
			haveShallowLine = (STEEP_DIRECTION_THRESHOLD * dist_01_04 <= dist_03_08) && (v[0] != v[2]) && (v[3] != v[2]);
			haveSteepLine   = (STEEP_DIRECTION_THRESHOLD * dist_03_08 <= dist_01_04) && (v[0] != v[6]) && (v[5] != v[6]);
			needBlend = (blendResult.y != BLEND_NONE);
			doLineBlend = (  blendResult.y >= BLEND_DOMINANT ||
						  !((blendResult.x != BLEND_NONE && !IsPixEqual(src[0], src[2])) ||
							(blendResult.z != BLEND_NONE && !IsPixEqual(src[0], src[6])) ||
							(IsPixEqual(src[2], src[1]) && IsPixEqual(src[1], src[8]) && IsPixEqual(src[8], src[7]) && IsPixEqual(src[7], src[6]) && !IsPixEqual(src[0], src[8])) ) );
	
			dist_01_04 = DistYCbCr(src[7], src[2]);
			dist_03_08 = DistYCbCr(src[1], src[6]);
			haveShallowLine = (STEEP_DIRECTION_THRESHOLD * dist_01_04 <= dist_03_08) && (v[0] != v[2]) && (v[3] != v[2]);
			haveSteepLine   = (STEEP_DIRECTION_THRESHOLD * dist_03_08 <= dist_01_04) && (v[0] != v[6]) && (v[5] != v[6]);
			needBlend = (blendResult.y != BLEND_NONE);
			doLineBlend = (  blendResult.y >= BLEND_DOMINANT ||
						  !((blendResult.x != BLEND_NONE && !IsPixEqual(src[0], src[2])) ||
							(blendResult.z != BLEND_NONE && !IsPixEqual(src[0], src[6])) ||
							(IsPixEqual(src[2], src[1]) && IsPixEqual(src[1], src[8]) && IsPixEqual(src[8], src[7]) && IsPixEqual(src[7], src[6]) && !IsPixEqual(src[0], src[8])) ) );
		
			blendPix = ( DistYCbCr(src[0], src[7]) <= DistYCbCr(src[0], src[1]) ) ? src[7] : src[1];
			dst[ 7] = mix(dst[ 7], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.250 : 0.000);
			dst[ 8] = mix(dst[ 8], blendPix, (needBlend && doLineBlend) ? ((haveSteepLine) ? 0.750 : ((haveShallowLine) ? 0.250 : 0.000)) : 0.000);
			dst[ 9] = mix(dst[ 9], blendPix, (needBlend && doLineBlend) ? ((!haveShallowLine && !haveSteepLine) ? 0.500 : 1.000) : 0.000);
			dst[10] = mix(dst[10], blendPix, (needBlend && doLineBlend) ? ((haveShallowLine) ? 0.750 : ((haveSteepLine) ? 0.250 : 0.000)) : 0.000);
			dst[11] = mix(dst[11], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.250 : 0.000);
			dst[20] = mix(dst[20], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.250 : 0.000);
			dst[21] = mix(dst[21], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.750 : 0.000);
			dst[22] = mix(dst[22], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 1.000 : 0.000);
			dst[23] = mix(dst[23], blendPix, (needBlend) ? ((doLineBlend) ? ((haveSteepLine) ? 1.000 : ((haveShallowLine) ? 0.750 : 0.500)) : 0.05652034508) : 0.000);
			dst[24] = mix(dst[24], blendPix, (needBlend) ? ((doLineBlend) ? 1.000 : 0.4236372243) : 0.000);
			dst[25] = mix(dst[25], blendPix, (needBlend) ? ((doLineBlend) ? 1.000 : 0.9711013910) : 0.000);
			dst[26] = mix(dst[26], blendPix, (needBlend) ? ((doLineBlend) ? 1.000 : 0.4236372243) : 0.000);
			dst[27] = mix(dst[27], blendPix, (needBlend) ? ((doLineBlend) ? ((haveShallowLine) ? 1.000 : ((haveSteepLine) ? 0.750 : 0.500)) : 0.05652034508) : 0.000);
			dst[28] = mix(dst[28], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 1.000 : 0.000);
			dst[29] = mix(dst[29], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.750 : 0.000);
			dst[30] = mix(dst[30], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.250 : 0.000);

			dist_01_04 = DistYCbCr(src[5], src[8]);
			dist_03_08 = DistYCbCr(src[7], src[4]);
			haveShallowLine = (STEEP_DIRECTION_THRESHOLD * dist_01_04 <= dist_03_08) && (v[0] != v[8]) && (v[1] != v[8]);
			haveSteepLine   = (STEEP_DIRECTION_THRESHOLD * dist_03_08 <= dist_01_04) && (v[0] != v[4]) && (v[3] != v[4]);
			needBlend = (blendResult.x != BLEND_NONE);
			doLineBlend = (  blendResult.x >= BLEND_DOMINANT ||
						  !((blendResult.w != BLEND_NONE && !IsPixEqual(src[0], src[8])) ||
							(blendResult.y != BLEND_NONE && !IsPixEqual(src[0], src[4])) ||
							(IsPixEqual(src[8], src[7]) && IsPixEqual(src[7], src[6]) && IsPixEqual(src[6], src[5]) && IsPixEqual(src[5], src[4]) && !IsPixEqual(src[0], src[6])) ) );
		
			blendPix = ( DistYCbCr(src[0], src[5]) <= DistYCbCr(src[0], src[7]) ) ? src[5] : src[7];
			dst[ 4] = mix(dst[ 4], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.250 : 0.000);
			dst[ 5] = mix(dst[ 5], blendPix, (needBlend && doLineBlend) ? ((haveSteepLine) ? 0.750 : ((haveShallowLine) ? 0.250 : 0.000)) : 0.000);
			dst[ 6] = mix(dst[ 6], blendPix, (needBlend && doLineBlend) ? ((!haveShallowLine && !haveSteepLine) ? 0.500 : 1.000) : 0.000);
			dst[ 7] = mix(dst[ 7], blendPix, (needBlend && doLineBlend) ? ((haveShallowLine) ? 0.750 : ((haveSteepLine) ? 0.250 : 0.000)) : 0.000);
			dst[ 8] = mix(dst[ 8], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.250 : 0.000);
			dst[35] = mix(dst[35], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.250 : 0.000);
			dst[16] = mix(dst[16], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.750 : 0.000);
			dst[17] = mix(dst[17], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 1.000 : 0.000);
			dst[18] = mix(dst[18], blendPix, (needBlend) ? ((doLineBlend) ? ((haveSteepLine) ? 1.000 : ((haveShallowLine) ? 0.750 : 0.500)) : 0.05652034508) : 0.000);
			dst[19] = mix(dst[19], blendPix, (needBlend) ? ((doLineBlend) ? 1.000 : 0.4236372243) : 0.000);
			dst[20] = mix(dst[20], blendPix, (needBlend) ? ((doLineBlend) ? 1.000 : 0.9711013910) : 0.000);
			dst[21] = mix(dst[21], blendPix, (needBlend) ? ((doLineBlend) ? 1.000 : 0.4236372243) : 0.000);
			dst[22] = mix(dst[22], blendPix, (needBlend) ? ((doLineBlend) ? ((haveShallowLine) ? 1.000 : ((haveSteepLine) ? 0.750 : 0.500)) : 0.05652034508) : 0.000);
			dst[23] = mix(dst[23], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 1.000 : 0.000);
			dst[24] = mix(dst[24], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.750 : 0.000);
			dst[25] = mix(dst[25], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.250 : 0.000);
		
		
			dist_01_04 = DistYCbCr(src[3], src[6]);
			dist_03_08 = DistYCbCr(src[5], src[2]);
			haveShallowLine = (STEEP_DIRECTION_THRESHOLD * dist_01_04 <= dist_03_08) && (v[0] != v[6]) && (v[7] != v[6]);
			haveSteepLine   = (STEEP_DIRECTION_THRESHOLD * dist_03_08 <= dist_01_04) && (v[0] != v[2]) && (v[1] != v[2]);
			needBlend = (blendResult.w != BLEND_NONE);
			doLineBlend = (  blendResult.w >= BLEND_DOMINANT ||
						  !((blendResult.z != BLEND_NONE && !IsPixEqual(src[0], src[6])) ||
							(blendResult.x != BLEND_NONE && !IsPixEqual(src[0], src[2])) ||
							(IsPixEqual(src[6], src[5]) && IsPixEqual(src[5], src[4]) && IsPixEqual(src[4], src[3]) && IsPixEqual(src[3], src[2]) && !IsPixEqual(src[0], src[4])) ) );
		
			blendPix = ( DistYCbCr(src[0], src[3]) <= DistYCbCr(src[0], src[5]) ) ? src[3] : src[5];
			dst[13] = mix(dst[13], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.250 : 0.000);
			dst[14] = mix(dst[14], blendPix, (needBlend && doLineBlend) ? ((haveSteepLine) ? 0.750 : ((haveShallowLine) ? 0.250 : 0.000)) : 0.000);
			dst[15] = mix(dst[15], blendPix, (needBlend && doLineBlend) ? ((!haveShallowLine && !haveSteepLine) ? 0.500 : 1.000) : 0.000);
			dst[ 4] = mix(dst[ 4], blendPix, (needBlend && doLineBlend) ? ((haveShallowLine) ? 0.750 : ((haveSteepLine) ? 0.250 : 0.000)) : 0.000);
			dst[ 5] = mix(dst[ 5], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.250 : 0.000);
			dst[30] = mix(dst[30], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.250 : 0.000);
			dst[31] = mix(dst[31], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 0.750 : 0.000);
			dst[32] = mix(dst[32], blendPix, (needBlend && doLineBlend && haveSteepLine) ? 1.000 : 0.000);
			dst[33] = mix(dst[33], blendPix, (needBlend) ? ((doLineBlend) ? ((haveSteepLine) ? 1.000 : ((haveShallowLine) ? 0.750 : 0.500)) : 0.05652034508) : 0.000);
			dst[34] = mix(dst[34], blendPix, (needBlend) ? ((doLineBlend) ? 1.000 : 0.4236372243) : 0.000);
			dst[35] = mix(dst[35], blendPix, (needBlend) ? ((doLineBlend) ? 1.000 : 0.9711013910) : 0.000);
			dst[16] = mix(dst[16], blendPix, (needBlend) ? ((doLineBlend) ? 1.000 : 0.4236372243) : 0.000);
			dst[17] = mix(dst[17], blendPix, (needBlend) ? ((doLineBlend) ? ((haveShallowLine) ? 1.000 : ((haveSteepLine) ? 0.750 : 0.500)) : 0.05652034508) : 0.000);
			dst[18] = mix(dst[18], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 1.000 : 0.000);
			dst[19] = mix(dst[19], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.750 : 0.000);
			dst[20] = mix(dst[20], blendPix, (needBlend && doLineBlend && haveShallowLine) ? 0.250 : 0.000);
		
		}
	
		vec2 f = fract(fTex * texSize);
		vec3 res = mix( mix( mix( mix( mix( mix(dst[20], dst[21], step(one_sixth, f.x) ), dst[22], step(two_sixth, f.x) ), mix( mix(dst[23], dst[24], step(four_sixth, f.x) ), dst[25], step(five_sixth, f.x) ), step(0.50, f.x) ),
									  mix( mix( mix(dst[19], dst[ 6], step(one_sixth, f.x) ), dst[ 7], step(two_sixth, f.x) ), mix( mix(dst[ 8], dst[ 9], step(four_sixth, f.x) ), dst[26], step(five_sixth, f.x) ), step(0.50, f.x) ), step(one_sixth, f.y) ),
									  mix( mix( mix(dst[18], dst[ 5], step(one_sixth, f.x) ), dst[ 0], step(two_sixth, f.x) ), mix( mix(dst[ 1], dst[10], step(four_sixth, f.x) ), dst[27], step(five_sixth, f.x) ), step(0.50, f.x) ), step(two_sixth, f.y) ),
							mix( mix( mix( mix( mix(dst[17], dst[ 4], step(one_sixth, f.x) ), dst[ 3], step(two_sixth, f.x) ), mix( mix(dst[ 2], dst[11], step(four_sixth, f.x) ), dst[28], step(five_sixth, f.x) ), step(0.50, f.x) ),
									  mix( mix( mix(dst[16], dst[15], step(one_sixth, f.x) ), dst[14], step(two_sixth, f.x) ), mix( mix(dst[13], dst[12], step(four_sixth, f.x) ), dst[29], step(five_sixth, f.x) ), step(0.50, f.x) ), step(four_sixth, f.y) ),
									  mix( mix( mix(dst[35], dst[34], step(one_sixth, f.x) ), dst[33], step(two_sixth, f.x) ), mix( mix(dst[32], dst[31], step(four_sixth, f.x) ), dst[30], step(five_sixth, f.x) ), step(0.50, f.x) ), step(five_sixth, f.y) ),
							 step(0.50, f.y) );
								 
		fragColor = vec4(res, check.a);
	}
	else
		fragColor = vec4(0.0);
}