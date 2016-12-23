#version 330

#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

in vec2 TexCoord0;
uniform sampler2D nmsSampler;

uniform vec2 sketchSize;


void main(void)
{
	vec3 centerColor = texture2D( nmsSampler, vec2(0.0, 0.0) + TexCoord0).rgb;
	float upSide = texture2D( nmsSampler, vec2(0.0, 0.0) + TexCoord0 + vec2(centerColor.g, centerColor.b)/ sketchSize.xy).r;
	float downSide = texture2D( nmsSampler, vec2(0.0, 0.0) + TexCoord0 + vec2(-centerColor.g, -centerColor.b)/ sketchSize.xy).r;

	lowp float maxValue = max(centerColor.r, upSide);
	maxValue = max(maxValue, downSide);

	gl_FragColor = vec4(centerColor * step(maxValue, centerColor.r), 1.0);
}
