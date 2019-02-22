/*
	ScaleHq fragment shader
	based on libretro ScaleHq shader
	https://github.com/libretro/glsl-shaders/tree/master/scalehq/shaders

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

in vec4 t1;
in vec4 t2;
in vec4 t3;
in vec4 t4;
in vec2 fTexCoord;

out vec4 fragColor;

#define MX 0.325
#define K -0.250
#define MAX_W 0.25
#define MIN_W -0.05
#define LUM_ADD 0.25

vec3 dt = vec3(1.0);

void main()
{
	vec3 c00 = texture(tex01, t1.xy).xyz; 
	vec3 c10 = texture(tex01, t1.zw).xyz; 
	vec3 c20 = texture(tex01, t2.xy).xyz; 
	vec3 c01 = texture(tex01, t4.zw).xyz; 
	vec3 c11 = texture(tex01, fTexCoord).xyz; 
	vec3 c21 = texture(tex01, t2.zw).xyz; 
	vec3 c02 = texture(tex01, t4.xy).xyz; 
	vec3 c12 = texture(tex01, t3.zw).xyz; 
	vec3 c22 = texture(tex01, t3.xy).xyz; 

	float md1 = dot(abs(c00 - c22), dt);
	float md2 = dot(abs(c02 - c20), dt);

	float w1 = dot(abs(c22 - c11), dt) * md2;
	float w2 = dot(abs(c02 - c11), dt) * md1;
	float w3 = dot(abs(c00 - c11), dt) * md2;
	float w4 = dot(abs(c20 - c11), dt) * md1;

	float t1 = w1 + w3;
	float t2 = w2 + w4;
	float ww = max(t1, t2) + 0.0001;

	c11 = (w1 * c00 + w2 * c20 + w3 * c22 + w4 * c02 + ww * c11) / (t1 + t2 + ww);

	float lc1 = K / (0.12 * dot(c10 + c12 + c11, dt) + LUM_ADD);
	float lc2 = K / (0.12 * dot(c01 + c21 + c11, dt) + LUM_ADD);

	w1 = clamp(lc1 * dot(abs(c11 - c10), dt) + MX, MIN_W, MAX_W);
	w2 = clamp(lc2 * dot(abs(c11 - c21), dt) + MX, MIN_W, MAX_W);
	w3 = clamp(lc1 * dot(abs(c11 - c12), dt) + MX, MIN_W, MAX_W);
	w4 = clamp(lc2 * dot(abs(c11 - c01), dt) + MX, MIN_W, MAX_W);

	fragColor = vec4(w1 * c10 + w2 * c21 + w3 * c12 + w4 * c01 + (1.0 - w1 - w2 - w3 - w4) * c11, 1.0);
}