/*
	SuperEagle fragment shader
	based on libretro SuperEagle shader
	https://github.com/libretro/glsl-shaders/blob/master/eagle/shaders

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
in vec4 t4;
in vec4 t5;
in vec4 t6;
in vec4 t7;
in vec4 t8;
in vec2 fTexCoord;

out vec4 fragColor;

vec3 dtt = vec3(65536.0,255.0,1.0);

float reduce(vec3 color)
{
	return dot(color, dtt);
}

int GET_RESULT(float A, float B, float C, float D)
{
   int x = 0; int y = 0; int r = 0;
   if (A == C) x+=1; else if (B == C) y+=1;
   if (A == D) x+=1; else if (B == D) y+=1;
   if (x <= 1) r+=1; 
   if (y <= 1) r-=1;
   return r;
} 

void main()
{
	vec3 C0 = texture(tex01, t1.xy).xyz; 
	vec3 C1 = texture(tex01, t1.zw).xyz;
	vec3 C2 = texture(tex01, t2.xy).xyz;
	vec3 D3 = texture(tex01, t2.zw).xyz;
	vec3 C3 = texture(tex01, t3.xy).xyz;
	vec3 C4 = texture(tex01, fTexCoord).xyz;
	vec3 C5 = texture(tex01, t3.zw).xyz;
	vec3 D4 = texture(tex01, t4.xy).xyz;
	vec3 C6 = texture(tex01, t4.zw).xyz;
	vec3 C7 = texture(tex01, t5.xy).xyz;
	vec3 C8 = texture(tex01, t5.zw).xyz;
	vec3 D5 = texture(tex01, t6.xy).xyz;
	vec3 D0 = texture(tex01, t6.zw).xyz;
	vec3 D1 = texture(tex01, t7.xy).xyz;
	vec3 D2 = texture(tex01, t7.zw).xyz;
	vec3 D6 = texture(tex01, t8.xy).xyz;

	vec3 p00,p10,p01,p11;

	float c0 = reduce(C0);float c1 = reduce(C1);
	float c2 = reduce(C2);float c3 = reduce(C3);
	float c4 = reduce(C4);float c5 = reduce(C5);
	float c6 = reduce(C6);float c7 = reduce(C7);
	float c8 = reduce(C8);float d0 = reduce(D0);
	float d1 = reduce(D1);float d2 = reduce(D2);
	float d3 = reduce(D3);float d4 = reduce(D4);
	float d5 = reduce(D5);float d6 = reduce(D6);

	if (c4 != c8) {
	  if (c7 == c5) {
		 p01 = p10 = C7;
		 if ((c6 == c7) || (c5 == c2)) {
			p00 = 0.25*(3.0*C7+C4);
		 } else {
			p00 = 0.5*(C4+C5);
		 }

		 if ((c5 == d4) || (c7 == d1)) {
			p11 = 0.25*(3.0*C7+C8);
		 } else {
			p11 = 0.5*(C7+C8);
		 }
	  } else {
		 p11 = 0.125*(6.0*C8+C7+C5);
		 p00 = 0.125*(6.0*C4+C7+C5);

		 p10 = 0.125*(6.0*C7+C4+C8);
		 p01 = 0.125*(6.0*C5+C4+C8);
	  }
	} else {
	  if (c7 != c5) {
		 p11 = p00 = C4;

		 if ((c1 == c4) || (c8 == d5)) {
			p01 = 0.25*(3.0*C4+C5);
		 } else {
			p01 = 0.5*(C4+C5);
		 }

		 if ((c8 == d2) || (c3 == c4)) {
			p10 = 0.25*(3.0*C4+C7);
		 } else {
			p10 = 0.5*(C7+C8);
		 }
	  } else {
		 int r = 0;
		 r += GET_RESULT(c5,c4,c6,d1);
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
	}

	vec2 fp = fract(fTexCoord * texSize);
	p10 = (fp.x < 0.50) ? (fp.y < 0.50 ? p00 : p10) : (fp.y < 0.50 ? p01: p11);

	fragColor = vec4(p10, 1.0);
}