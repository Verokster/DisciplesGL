/*
	MIT License

	Copyright (c) 2020 Oleksiy Ryabchun

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

uniform sampler2D tex01;
#ifdef DOUBLE
uniform sampler2D tex02;
#endif
uniform vec2 texSize;
#if defined(LEV_IN_RGB) || defined(LEV_IN_A)
#define LEV_IN
#endif
#if defined(LEV_GAMMA_RGB) || defined(LEV_GAMMA_A)
#define LEV_GAMMA
#endif
#if defined(LEV_OUT_RGB) || defined(LEV_OUT_A)
#define LEV_OUT
#endif
#if defined(LEV_IN) || defined(LEV_GAMMA) || defined(LEV_OUT)
#define LEVELS
#endif
#if defined(LEV_HUE_L) || defined(LEV_HUE_R)
#define LEV_HUE
#endif
#if defined(LEV_HUE) || defined(LEV_SAT)
#define SATHUE
uniform vec2 satHue;
#endif
#ifdef LEV_IN
uniform vec4 in_left;
uniform vec4 in_right;
#endif
#ifdef LEV_GAMMA
uniform vec4 gamma;
#endif
#ifdef LEV_OUT
uniform vec4 out_left;
uniform vec4 out_right;
#endif

#if __VERSION__ >= 130
	#define COMPAT_IN in
	#define COMPAT_TEXTURE texture
	out vec4 FRAG_COLOR;
#else
	#define COMPAT_IN varying 
	#define COMPAT_TEXTURE texture2D
	#define FRAG_COLOR gl_FragColor
#endif

#ifdef DOUBLE
COMPAT_IN vec4 fTex;
#else
COMPAT_IN vec2 fTex;
#endif

vec4 hermite(sampler2D tex, vec2 coord) {
	vec2 uv = coord * texSize - 0.5;
	vec2 texel = floor(uv) + 0.5;
	vec2 t = fract(uv);

	uv = texel + t * t * t * (t * (t * 6.0 - 15.0) + 10.0);

	return COMPAT_TEXTURE(tex, uv / texSize);
}

#ifdef SATHUE
vec3 saturate(vec3 color) {
#ifdef LEV_HUE
#ifdef LEV_HUE_L
	color = color.brg + 0.5 * satHue.y * (color - color.gbr + satHue.y * (color.gbr - 5.0 * color.brg + 4.0 * color + 3.0 * satHue.y * (color.brg - color)));
#else
	color = color + 0.5 * satHue.y * (color.gbr - color.brg + satHue.y * (color.brg - 5.0 * color + 4.0 * color.gbr + 3.0 * satHue.y * (color - color.gbr)));
#endif
#endif
#ifdef LEV_SAT
	float s = dot(color, vec3(1.0)) / 3.0;
	color = (color - s) * satHue.x + s;
#endif
	return color;
}
#endif

#ifdef LEVELS
vec3 levels(vec3 color) {
#ifdef LEV_IN_RGB
	color = clamp((color - in_left.rgb) / (in_right.rgb - in_left.rgb), 0.0, 1.0);
#endif
#ifdef LEV_GAMMA_RGB
	color = pow(color, gamma.rgb);
#endif
#ifdef LEV_OUT_RGB
	color = color * (out_right.rgb - out_left.rgb) + out_left.rgb;
#endif
#ifdef LEV_IN_A
	color = clamp((color - in_left.a) / (in_right.a - in_left.a), 0.0, 1.0);
#endif
#ifdef LEV_GAMMA_A
	color = pow(color, gamma.aaa);
#endif
#ifdef LEV_OUT_A
	color = color * (out_right.a - out_left.a) + out_left.a;
#endif
	return color;
}
#endif

void main() {
#ifdef DOUBLE
	vec4 front = hermite(tex01, fTex.rg);
	vec4 back = hermite(tex02, fTex.ba);
	vec3 color = mix(back.rgb, front.rgb, front.a);
#else
	vec3 color = hermite(tex01, fTex).rgb;
#endif

#ifdef SATHUE
	color = saturate(color);
#endif
#ifdef LEVELS
	color = levels(color);
#endif
	
	FRAG_COLOR = vec4(color, 1.0);
}