/*
	xSal fragment shader
	based on libretro xSal shader
	https://github.com/libretro/glsl-shaders/tree/master/xsal/shaders

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

in vec2 fTexCoord;

out vec4 fragColor;

void main()
{
    float dx = pow(texSize.x, -1.0) * 0.25;
    float dy = pow(texSize.y, -1.0) * 0.25;
    vec3  dt = vec3(1.0, 1.0, 1.0);
 
    vec3 c00 = texture(tex01, fTexCoord + vec2(-dx, -dy)).xyz;
    vec3 c20 = texture(tex01, fTexCoord + vec2( dx, -dy)).xyz;
    vec3 c02 = texture(tex01, fTexCoord + vec2(-dx,  dy)).xyz;
    vec3 c22 = texture(tex01, fTexCoord + vec2( dx,  dy)).xyz;

    float m1 = dot(abs(c00 - c22), dt) + 0.001;
    float m2 = dot(abs(c02 - c20), dt) + 0.001;

    fragColor = vec4((m1*(c02 + c20) + m2*(c22 + c00))/(2.0*(m1 + m2)), 1.0);
}