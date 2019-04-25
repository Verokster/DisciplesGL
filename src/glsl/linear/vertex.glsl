uniform mat4 mvp;

#if __VERSION__ >= 130
	#define COMPAT_IN in
	#define COMPAT_OUT out
	precision mediump float;
#else
	#define COMPAT_IN attribute 
	#define COMPAT_OUT varying 
#endif

COMPAT_IN vec2 vCoord;
COMPAT_IN vec2 vTexCoord;
COMPAT_OUT vec2 fTexCoord;

void main() {
	gl_Position = mvp * vec4(vCoord, 0.0, 1.0);
	fTexCoord = vTexCoord;
}