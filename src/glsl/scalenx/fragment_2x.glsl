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
	vec3 B = texture(tex01, t1.xy).xyz;
	vec3 D = texture(tex01, t1.zw).xyz;
	vec3 E = texture(tex01, fTexCoord).xyz;
	vec3 F = texture(tex01, t2.xy).xyz;
	vec3 H = texture(tex01, t2.zw).xyz;

	vec3 E0 = eq(B,D) ? B : E;
	vec3 E1 = eq(B,F) ? B : E;
	vec3 E2 = eq(H,D) ? H : E;
	vec3 E3 = eq(H,F) ? H : E;

	vec2 fp = floor(2.0 * fract(fTexCoord * texSize));
	fragColor = vec4(neq(B,H) && neq(D,F) ? (fp.y == 0. ? (fp.x == 0. ? E0 : E1) : (fp.x == 0. ? E2 : E3)) : E, 1.0);
}