/*
	ScaleNx fragment shader
	based on libretro ScaleNx shader
	https://github.com/libretro/glsl-shaders/blob/master/scalenx/shaders

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

in vec2 fTex;
out vec4 fragColor;

bool eq(vec4 A, vec4 B) {
	return (A==B);
}

bool neq(vec4 A, vec4 B) {
	return (A!=B);
}

void main() {
	if (eq(texture(tex01, fTex), texture(tex02, fTex)))
		discard;

	vec2 texel = floor(fTex * texSize) + 0.5;

	#define TEX(x, y) texture(tex01, (texel + vec2(x, y)) / texSize)

	vec4 B = TEX( 0.0, -1.0);
	vec4 D = TEX(-1.0,  0.0);
	vec4 E = TEX( 0.0,  0.0);
	vec4 F = TEX( 1.0,  0.0);
	vec4 H = TEX( 0.0,  1.0);

	vec4 E0 = eq(B,D) ? B : E;
	vec4 E1 = eq(B,F) ? B : E;
	vec4 E2 = eq(H,D) ? H : E;
	vec4 E3 = eq(H,F) ? H : E;

	vec2 fp = floor(2.0 * fract(fTex * texSize));
	fragColor = neq(B,H) && neq(D,F) ? (fp.y == 0. ? (fp.x == 0. ? E0 : E1) : (fp.x == 0. ? E2 : E3)) : E;
}