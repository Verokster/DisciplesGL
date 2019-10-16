/*
	Cubic fragment shader
	based on libretro Cubic shader
	https://github.com/libretro/glsl-shaders/tree/master/cubic/shaders

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

uniform sampler2D tex01;
uniform vec2 texSize;

#if __VERSION__ >= 130
	#define COMPAT_IN in
	#define COMPAT_TEXTURE texture
	out vec4 FRAG_COLOR;
#else
	#define COMPAT_IN varying 
	#define COMPAT_TEXTURE texture2D
	#define FRAG_COLOR gl_FragColor
#endif

COMPAT_IN vec2 fTex;

vec4 cubic(sampler2D tex, vec2 coord)
{
	vec2 uv = coord * texSize - 0.5;
	vec2 texel = floor(uv) - 0.5;
	vec2 t = fract(uv);

	vec2 t2 = t * t;
	vec2 t3 = t2 * t;

	vec4 xs = vec4(1.0, t.x, t2.x, t3.x);
	vec4 ys = vec4(1.0, t.y, t2.y, t3.y);

	const vec4 p0 = vec4(+0.0, -0.5, +1.0, -0.5);
	const vec4 p1 = vec4(+1.0,  0.0, -2.5, +1.5);
	const vec4 p2 = vec4(+0.0, +0.5, +2.0, -1.5);
	const vec4 p3 = vec4(+0.0,  0.0, -0.5, +0.5);

	vec4 x = vec4(dot(xs, p0), dot(xs, p1), dot(xs, p2), dot(xs, p3));
	vec4 y = vec4(dot(ys, p0), dot(ys, p1), dot(ys, p2), dot(ys, p3));

	vec4 color = x.r * y.r * COMPAT_TEXTURE(tex, (texel + vec2(0.0, 0.0)) / texSize) +
		x.g * y.r * COMPAT_TEXTURE(tex, (texel + vec2(1.0, 0.0)) / texSize) +
		x.b * y.r * COMPAT_TEXTURE(tex, (texel + vec2(2.0, 0.0)) / texSize) +
		x.a * y.r * COMPAT_TEXTURE(tex, (texel + vec2(3.0, 0.0)) / texSize);

	color += x.r * y.g * COMPAT_TEXTURE(tex, (texel + vec2(0.0, 1.0)) / texSize) +
		x.g * y.g * COMPAT_TEXTURE(tex, (texel + vec2(1.0, 1.0)) / texSize) +
		x.b * y.g * COMPAT_TEXTURE(tex, (texel + vec2(2.0, 1.0)) / texSize) +
		x.a * y.g * COMPAT_TEXTURE(tex, (texel + vec2(3.0, 1.0)) / texSize);

	color += x.r * y.b * COMPAT_TEXTURE(tex, (texel + vec2(0.0, 2.0)) / texSize) +
		x.g * y.b * COMPAT_TEXTURE(tex, (texel + vec2(1.0, 2.0)) / texSize) +
		x.b * y.b * COMPAT_TEXTURE(tex, (texel + vec2(2.0, 2.0)) / texSize) +
		x.a * y.b * COMPAT_TEXTURE(tex, (texel + vec2(3.0, 2.0)) / texSize);

	return color + x.r * y.a * COMPAT_TEXTURE(tex, (texel + vec2(0.0, 3.0)) / texSize) +
		x.g * y.a * COMPAT_TEXTURE(tex, (texel + vec2(1.0, 3.0)) / texSize) +
		x.b * y.a * COMPAT_TEXTURE(tex, (texel + vec2(2.0, 3.0)) / texSize) +
		x.a * y.a * COMPAT_TEXTURE(tex, (texel + vec2(3.0, 3.0)) / texSize);
}

void main() {
	FRAG_COLOR = cubic(tex01, fTex);
} 