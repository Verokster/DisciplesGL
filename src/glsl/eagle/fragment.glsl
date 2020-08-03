/*
	SuperEagle fragment shader
	based on libretro SuperEagle shader
	https://github.com/libretro/glsl-shaders/blob/master/eagle/shaders

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

const vec4 dtt = vec4(65536.0,255.0,1.0,0.0);

float reduce(vec4 color) {
	return dot(color, dtt);
}

int GET_RESULT(float A, float B, float C, float D) {
	int x = 0; int y = 0; int r = 0;
	if (A == C) x+=1; else if (B == C) y+=1;
	if (A == D) x+=1; else if (B == D) y+=1;
	if (x <= 1) r+=1; 
	if (y <= 1) r-=1;
	return r;
}

void main() {
	if (texture(tex01, fTex) == texture(tex02, fTex))
		discard;

	vec2 texel = floor(fTex * texSize) + 0.5;

	#define TEX(x, y) texture(tex01, (texel + vec2(x, y)) / texSize)

	vec4 C0 = TEX(-1.0, -1.0);
	vec4 C1 = TEX(-1.0,  0.0);
	vec4 C2 = TEX( 1.0, -1.0);
	vec4 D3 = TEX( 2.0, -1.0);
	vec4 C3 = TEX(-1.0,  0.0);
	vec4 C4 = TEX( 0.0,  0.0);
	vec4 C5 = TEX( 1.0,  0.0);
	vec4 D4 = TEX( 2.0,  0.0);
	vec4 C6 = TEX(-1.0,  1.0);
	vec4 C7 = TEX( 0.0,  1.0);
	vec4 C8 = TEX( 1.0,  1.0);
	vec4 D5 = TEX( 0.0,  1.0);
	vec4 D0 = TEX( 1.0,  1.0);
	vec4 D1 = TEX( 0.0,  2.0);
	vec4 D2 = TEX( 1.0,  2.0);
	vec4 D6 = TEX( 2.0,  2.0);

	vec4 p00,p10,p01,p11;

	float c0 = reduce(C0); float c1 = reduce(C1);
	float c2 = reduce(C2); float c3 = reduce(C3);
	float c4 = reduce(C4); float c5 = reduce(C5);
	float c6 = reduce(C6); float c7 = reduce(C7);
	float c8 = reduce(C8); float d0 = reduce(D0);
	float d1 = reduce(D1); float d2 = reduce(D2);
	float d3 = reduce(D3); float d4 = reduce(D4);
	float d5 = reduce(D5); float d6 = reduce(D6);

	if (c4 != c8) {
		if (c7 == c5) {
			p01 = p10 = C7;
			p00 = c6 == c7 || c5 == c2 ? 0.25*(3.0*C7+C4) : 0.5*(C4+C5);
			p11 = c5 == d4 || c7 == d1 ? 0.25*(3.0*C7+C8) : 0.5*(C7+C8);
		} else {
			p11 = 0.125*(6.0*C8+C7+C5);
			p00 = 0.125*(6.0*C4+C7+C5);
			p10 = 0.125*(6.0*C7+C4+C8);
			p01 = 0.125*(6.0*C5+C4+C8);
		}
	} else if (c7 != c5) {
		p11 = p00 = C4;
		p01 = c1 == c4 || c8 == d5 ? 0.25*(3.0*C4+C5) : 0.5*(C4+C5);
		p10 = c8 == d2 || c3 == c4 ? 0.25*(3.0*C4+C7) : 0.5*(C7+C8);
	} else {
		int r = GET_RESULT(c5,c4,c6,d1);
		r += GET_RESULT(c5,c4,c3,c1);
		r += GET_RESULT(c5,c4,d2,d5);
		r += GET_RESULT(c5,c4,c2,d4);

		if (r > 0) {
			p01 = p10 = C7;
			p00 = p11 = 0.5*(C4+C5);
		} else if (r < 0) {
			p11 = p00 = C4;
			p01 = p10 = 0.5*(C4+C5);
		} else {
			p11 = p00 = C4;
			p01 = p10 = C7;
		}
	}

	vec2 fp = fract(fTex * texSize);
	fragColor = fp.x < 0.50 ? (fp.y < 0.50 ? p00 : p10) : (fp.y < 0.50 ? p01: p11);
}