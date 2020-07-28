/*
	ScaleHq fragment shader
	based on libretro ScaleHq shader
	https://github.com/libretro/glsl-shaders/tree/master/scalehq/shaders

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

#define MX 1.0
#define K -1.10
#define MAX_W 0.75
#define MIN_W 0.03
#define LUM_ADD 0.33

const vec4 dt = vec4(1.0);

void main() {
	if (texture(tex01, fTex) == texture(tex02, fTex))
		discard;

	#define TEX(x, y) texture(tex01, (floor(fTex * texSize + vec2(x, y)) + 0.5) / texSize)

	vec4 c  = TEX( 0.0,   0.0);

	vec4 i1 = TEX(-0.25, -0.25); 
	vec4 i2 = TEX( 0.25, -0.25); 
	vec4 i3 = TEX( 0.25,  0.25); 
	vec4 i4 = TEX(-0.25,  0.25); 

	vec2 texel;

	#define TEX2(x, y) texture(tex01, (texel + vec2(x, y)) / texSize)

	texel = floor(fTex * texSize - 0.5) + 0.5;
	vec4 o1 = TEX2(0.0, 0.0); 
	vec4 o3 = TEX2(1.0, 1.0); 
	vec4 o2 = TEX2(1.0, 0.0);
	vec4 o4 = TEX2(0.0, 1.0);

	texel = floor(fTex * texSize - vec2(0.0, 0.5)) + 0.5;
	vec4 s1 = TEX2(0.0, 0.0); 
	vec4 s3 = TEX2(0.0, 1.0);

	texel = floor(fTex * texSize - vec2(0.5, 0.0)) + 0.5;
	vec4 s2 = TEX2(1.0, 0.0); 
	vec4 s4 = TEX2(0.0, 0.0);

	float ko1=dot(abs(o1-c),dt);
	float ko2=dot(abs(o2-c),dt);
	float ko3=dot(abs(o3-c),dt);
	float ko4=dot(abs(o4-c),dt);

	float k1=min(dot(abs(i1-i3),dt),max(ko1,ko3));
	float k2=min(dot(abs(i2-i4),dt),max(ko2,ko4));

	float w1 = k2; if(ko3<ko1) w1*=ko3/ko1;
	float w2 = k1; if(ko4<ko2) w2*=ko4/ko2;
	float w3 = k2; if(ko1<ko3) w3*=ko1/ko3;
	float w4 = k1; if(ko2<ko4) w4*=ko2/ko4;

	c=(w1*o1+w2*o2+w3*o3+w4*o4+0.001*c)/(w1+w2+w3+w4+0.001);
	w1 = K*dot(abs(i1-c)+abs(i3-c),dt)/(0.125*dot(i1+i3,dt)+LUM_ADD);
	w2 = K*dot(abs(i2-c)+abs(i4-c),dt)/(0.125*dot(i2+i4,dt)+LUM_ADD);
	w3 = K*dot(abs(s1-c)+abs(s3-c),dt)/(0.125*dot(s1+s3,dt)+LUM_ADD);
	w4 = K*dot(abs(s2-c)+abs(s4-c),dt)/(0.125*dot(s2+s4,dt)+LUM_ADD);

	w1 = clamp(w1+MX,MIN_W,MAX_W); 
	w2 = clamp(w2+MX,MIN_W,MAX_W);
	w3 = clamp(w3+MX,MIN_W,MAX_W); 
	w4 = clamp(w4+MX,MIN_W,MAX_W);

	fragColor = vec4((w1*(i1+i3)+w2*(i2+i4)+w3*(s1+s3)+w4*(s2+s4)+c)/(2.0*(w1+w2+w3+w4)+1.0));
}