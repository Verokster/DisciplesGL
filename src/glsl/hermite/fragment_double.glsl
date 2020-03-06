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
uniform sampler2D tex02;
uniform vec2 texSize;
uniform float hue;
uniform float sat;
uniform vec4 in_left;
uniform vec4 in_right;
uniform vec4 gamma;
uniform vec4 out_left;
uniform vec4 out_right;

#if __VERSION__ >= 130
	#define COMPAT_IN in
	#define COMPAT_TEXTURE texture
	out vec4 FRAG_COLOR;
#else
	#define COMPAT_IN varying 
	#define COMPAT_TEXTURE texture2D
	#define FRAG_COLOR gl_FragColor
#endif

COMPAT_IN vec4 fTex;

vec4 hermite(sampler2D tex, vec2 coord)
{
	vec2 uv = coord * texSize - 0.5;
	vec2 texel = floor(uv) + 0.5;
	vec2 t = fract(uv);

	uv = texel + t * t * t * (t * (t * 6.0 - 15.0) + 10.0);

	return COMPAT_TEXTURE(tex, uv / texSize);
}

vec3 satHue(vec3 color)
{
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

vec3 levels(vec3 color)
{
	color = clamp((color - in_left.rgb) / (in_right.rgb - in_left.rgb), 0.0, 1.0);
	color = pow(color, gamma.rgb);
	color = clamp(color * (out_right.rgb - out_left.rgb) + out_left.rgb, 0.0, 1.0);

	color = clamp((color - in_left.aaa) / (in_right.aaa - in_left.aaa), 0.0, 1.0);
	color = pow(color, gamma.aaa);
	return clamp(color * (out_right.aaa - out_left.aaa) + out_left.aaa, 0.0, 1.0);
}

void main() {
	vec4 front = hermite(tex01, fTex.rg);
	vec4 back = hermite(tex02, fTex.ba);
	vec3 color = mix(back.rgb, front.rgb, front.a);

	color = satHue(color);
	FRAG_COLOR = vec4(levels(color), 1.0);
}