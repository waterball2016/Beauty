#version 330

#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

in vec2 TexCoord0;
uniform sampler2D edgeSampler;

uniform vec2 sketchSize;

vec3 grad(float edgeStrengh)
{
	float topLeft = texture2D( edgeSampler, vec2(0.0, 0.0) + TexCoord0 + vec2(-1, 1)/ sketchSize.xy).r;
	float top = texture2D( edgeSampler, vec2(0.0, 0.0) + TexCoord0 + vec2(0, 1)/ sketchSize.xy).r;
	float topRight = texture2D( edgeSampler, vec2(0.0, 0.0) + TexCoord0 + vec2(1, 1)/ sketchSize.xy).r;
	float bottomLeft = texture2D( edgeSampler, vec2(0.0, 0.0) + TexCoord0 + vec2(-1, -1)/ sketchSize.xy).r;
	float bottom = texture2D( edgeSampler, vec2(0.0, 0.0) + TexCoord0 + vec2(0, -1)/ sketchSize.xy).r;
	float bottomRight = texture2D( edgeSampler, vec2(0.0, 0.0) + TexCoord0 + vec2(1, -1)/ sketchSize.xy).r;
	float left = texture2D( edgeSampler, vec2(0.0, 0.0) + TexCoord0 + vec2(-1, 0)/ sketchSize.xy).r;
	float right = texture2D( edgeSampler, vec2(0.0, 0.0) + TexCoord0 + vec2(1, 0)/ sketchSize.xy).r;

	float insHor = topRight + bottomRight + 2*right - topLeft - bottomLeft - 2*left;
	float insVer = topRight + topLeft + 2*top - bottomRight - bottomLeft - 2*bottom;
	float length = length(vec2(insHor, insVer));
	float ins = length * edgeStrengh;

	float dirHor = step(length, 2*abs(insVer));
	float dirVer = step(length, 2*abs(insVer));

	return vec3(ins, sign(insHor) * dirHor, sign(insVer) * dirVer);
}

void main(void)
{
	vec3 grad = grad(1);
	gl_FragColor = vec4(grad, 1.0);
}
