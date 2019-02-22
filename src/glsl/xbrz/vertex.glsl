/*
	xBRZ vertex shader
	based on libretro xBRZ shader
	https://github.com/libretro/glsl-shaders/tree/master/xbrz/shaders

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

uniform mat4 mvp;
uniform sampler2D tex01;
uniform vec2 texSize;

in vec2 vCoord;
in vec2 vTexCoord;

out vec4 t1;
out vec4 t2;
out vec4 t3;
out vec4 t4;
out vec4 t5;
out vec4 t6;
out vec4 t7;
out vec2 fTexCoord;

void main() {
	gl_Position = mvp * vec4(vCoord, 0.0, 1.0);
	
	vec2 d = 1.0 / texSize;
	t1 = vTexCoord.xxxy + vec4(-d.x, 0.0, d.x,-2.0 * d.y);
	t2 = vTexCoord.xxxy + vec4(-d.x, 0.0, d.x,-d.y);
	t3 = vTexCoord.xxxy + vec4(-d.x, 0.0, d.x, 0.0);
	t4 = vTexCoord.xxxy + vec4(-d.x, 0.0, d.x, d.y);
	t5 = vTexCoord.xxxy + vec4(-d.x, 0.0, d.x, 2.0 * d.y);
	t6 = vTexCoord.xyyy + vec4(-2.0 * d.x,-d.y, 0.0, d.y);
	t7 = vTexCoord.xyyy + vec4( 2.0 * d.x,-d.y, 0.0, d.y);
	
	fTexCoord = vTexCoord * texSize;
}