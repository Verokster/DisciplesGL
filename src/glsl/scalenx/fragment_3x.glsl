/*
	ScaleNx fragment shader
	based on libretro ScaleNx shader
	https://github.com/libretro/glsl-shaders/blob/master/scalenx/shaders

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
uniform vec2 texSize;

in vec4 t1;
in vec4 t2;
in vec4 t3;
in vec2 fTexCoord;

out vec4 fragColor;

bool eq(vec3 A, vec3 B){
	return (A==B);
}

bool neq(vec3 A, vec3 B){
	return (A!=B);
}

void main()
{
	vec3 A = texture(tex01, t1.xw).xyz;
	vec3 B = texture(tex01, t1.yw).xyz;
	vec3 C = texture(tex01, t1.zw).xyz;
	vec3 D = texture(tex01, t2.xw).xyz;
	vec3 E = texture(tex01, t2.yw).xyz;
	vec3 F = texture(tex01, t2.zw).xyz;
	vec3 G = texture(tex01, t3.xw).xyz;
	vec3 H = texture(tex01, t3.yw).xyz;
	vec3 I = texture(tex01, t3.zw).xyz;

	bool eqBD = eq(B,D), eqBF = eq(B,F), eqHD = eq(H,D), eqHF = eq(H,F), neqEA = neq(E,A), neqEC = neq(E,C), neqEG = neq(E,G), neqEI = neq(E,I); 

	vec3 E0 = eqBD ? B : E;
	vec3 E1 = eqBD && neqEC || eqBF && neqEA ? B : E;
	vec3 E2 = eqBF ? B : E;
	vec3 E3 = eqBD && neqEG || eqHD && neqEA ? D : E;
	vec3 E5 = eqBF && neqEI || eqHF && neqEC ? F : E;
	vec3 E6 = eqHD ? H : E;
	vec3 E7 = eqHD && neqEI || eqHF && neqEG ? H : E;
	vec3 E8 = eqHF ? H : E;

	vec2 fp = floor(3.0 * fract(fTexCoord));
	fragColor = vec4(neq(B,H) && neq(D,F) ? (fp.y == 0. ? (fp.x == 0. ? E0 : fp.x == 1. ? E1 : E2) : (fp.y == 1. ? (fp.x == 0. ? E3 : fp.x == 1. ? E : E5) : (fp.x == 0. ? E6 : fp.x == 1. ? E7 : E8))) : E, 1.0);
}