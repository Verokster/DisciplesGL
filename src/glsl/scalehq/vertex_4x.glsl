/*
	ScaleHq vertex shader
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
out vec2 fTexCoord;

void main() {
	gl_Position = mvp * vec4(vCoord, 0.0, 1.0);
	
	vec2 xy = 0.5 / texSize;
	vec2 dg1 = vec2( xy.x, xy.y);
	vec2 dg2 = vec2(-xy.x, xy.y);
	vec2 sd1 = dg1 * 0.5;
	vec2 sd2 = dg2 * 0.5;
	vec2 ddx = vec2( xy.x, 0.0);
	vec2 ddy = vec2( 0.0, xy.y);
	t1 = vec4(vTexCoord - sd1, vTexCoord - ddy);
	t2 = vec4(vTexCoord - sd2, vTexCoord + ddx);
	t3 = vec4(vTexCoord + sd1, vTexCoord + ddy);
	t4 = vec4(vTexCoord + sd2, vTexCoord - ddx);
	t5 = vec4(vTexCoord - dg1, vTexCoord - dg2);
	t6 = vec4(vTexCoord + dg1, vTexCoord + dg2);
	
	fTexCoord = vTexCoord;
}