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
#ifdef LEVELS
uniform float hue;
uniform float sat;
uniform vec4 in_left;
uniform vec4 in_right;
uniform vec4 gamma;
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

#define M_PI 3.1415926535897932384626433832795

vec3 weight(float a) {
	vec3 s = max(abs(2.0 * M_PI * vec3(a - 1.5, a - 0.5, a + 0.5)), 1e-5);
	return sin(s) * sin(s / 3.0) / (s * s);
}

vec4 lsum(sampler2D tex, float x[6], float y, vec3 y1, vec3 y2)
{
	vec4 v1 = mat3x4(
		COMPAT_TEXTURE(tex, vec2(x[0], y)),
		COMPAT_TEXTURE(tex, vec2(x[2], y)),
		COMPAT_TEXTURE(tex, vec2(x[4], y))) * y1;
		
	vec4 v2 = mat3x4(
		COMPAT_TEXTURE(tex, vec2(x[1], y)),
		COMPAT_TEXTURE(tex, vec2(x[3], y)),
		COMPAT_TEXTURE(tex, vec2(x[5], y))) * y2;

	return v1 + v2;
}

vec4 lanczos(sampler2D tex, vec2 coord) {
	vec2 stp = 1.0 / texSize;
	vec2 uv = coord + stp * 0.5;
	vec2 f = fract(uv / stp);

	vec3 y1 = weight(0.5 - f.x * 0.5);
	vec3 y2 = weight(1.0 - f.x * 0.5);
	vec3 x1 = weight(0.5 - f.y * 0.5);
	vec3 x2 = weight(1.0 - f.y * 0.5);

	const vec3 one = vec3(1.0);
	float xsum = dot(x1, one) + dot(x2, one);
	float ysum = dot(y1, one) + dot(y2, one);
	
	x1 /= xsum;
	x2 /= xsum;
	y1 /= ysum;
	y2 /= ysum;
	
	vec2 pos = (-0.5 - f) * stp + uv;
	float x[6] = float[](pos.x - stp.x * 2.0, pos.x - stp.x, pos.x, pos.x + stp.x, pos.x + stp.x * 2.0, pos.x + stp.x * 3.0);
	float y[6] = float[](pos.y - stp.y * 2.0, pos.y - stp.y, pos.y, pos.y + stp.y, pos.y + stp.y * 2.0, pos.y + stp.y * 3.0);

	vec4 v1 = mat3x4(
		lsum(tex, x, y[0], y1, y2),
		lsum(tex, x, y[2], y1, y2),
		lsum(tex, x, y[4], y1, y2)) * x1;
		
	vec4 v2 = mat3x4(
		lsum(tex, x, y[1], y1, y2),
		lsum(tex, x, y[3], y1, y2),
		lsum(tex, x, y[5], y1, y2)) * x2;

	return v1 + v2;
}

#ifdef DOUBLE
vec4 hermite(sampler2D tex, vec2 coord) {
	vec2 uv = coord * texSize - 0.5;
	vec2 texel = floor(uv) + 0.5;
	vec2 t = fract(uv);

	uv = texel + t * t * t * (t * (t * 6.0 - 15.0) + 10.0);

	return COMPAT_TEXTURE(tex, uv / texSize);
}
#endif

#ifdef LEVELS
vec3 satHue(vec3 color) {
	const mat3 mrgb = mat3(	  1.0,    1.0,    1.0,
							0.956, -0.272, -1.107,
							0.621, -0.647,  1.705 );

	const mat3 myiq = mat3(	0.299,  0.596,  0.211,
							0.587, -0.274, -0.523,
							0.114, -0.321,  0.311 );

	float su = sat * cos(hue);
	float sw = sat * sin(hue);

	mat3 mhsv = mat3(1.0, 0.0,  0.0,
					 0.0, su, sw,
					 0.0, -sw,  su );

	return mrgb * mhsv * myiq * color;
}

vec3 levels(vec3 color) {
	color = clamp((color - in_left.rgb) / (in_right.rgb - in_left.rgb), 0.0, 1.0);
	color = pow(color, gamma.rgb);
	color = clamp(color * (out_right.rgb - out_left.rgb) + out_left.rgb, 0.0, 1.0);

	color = clamp((color - in_left.aaa) / (in_right.aaa - in_left.aaa), 0.0, 1.0);
	color = pow(color, gamma.aaa);
	return clamp(color * (out_right.aaa - out_left.aaa) + out_left.aaa, 0.0, 1.0);
}
#endif

void main() {
#ifdef DOUBLE
	vec4 front = lanczos(tex01, fTex.rg);
	vec4 back = hermite(tex02, fTex.ba);
	vec3 color = mix(back.rgb, front.rgb, front.a);
#else
	vec3 color = lanczos(tex01, fTex).rgb;
#endif

#ifdef LEVELS
	color = satHue(color);
	color = levels(color);
#endif
	
	FRAG_COLOR = vec4(color, 1.0);
}