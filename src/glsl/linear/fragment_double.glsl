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
uniform vec3 minLevel;
uniform vec3 maxLevel;
uniform vec3 gamma;
uniform float saturation;
uniform float hueShift;

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

vec3 levels(vec3 color, vec3 minInput, vec3 maxInput, vec3 adjustment)
{
	return pow(min(max(color - minInput, vec3(0.0)) / (maxInput - minInput), vec3(1.0)), vec3(1.0 / adjustment));
}

vec3 adjustSaturation(vec3 color, float adjustment)
{
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    vec3 intensity = vec3(dot(color, W));
    return mix(intensity, color, adjustment);
}

vec3 shiftHue(vec3 color, float adjustment)
{
    vec3 P = vec3(0.55735) * dot(vec3(0.55735), color);
    vec3 U = color - P;
    vec3 V = cross(vec3(0.55735), U);    
    return U * cos(adjustment * 6.2832) + V * sin(adjustment * 6.2832) + P;
}

void main() {
	vec4 front = COMPAT_TEXTURE(tex01, fTex.rg);
	vec4 back = COMPAT_TEXTURE(tex02, fTex.ba);
	vec3 color = mix(back.rgb, front.rgb, front.a);

	color = levels(color, minLevel, maxLevel, gamma);
	color = adjustSaturation(color, saturation);
	color = shiftHue(color, hueShift);

	FRAG_COLOR = vec4(color, 1.0);
}