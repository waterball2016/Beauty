#version 330

#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

in vec2 TexCoord0;
uniform sampler2D sharpSampler;
uniform vec2 sketchSize;

void main(void)
{
	float lambda = 0.3;
	vec3 top = texture2D( sharpSampler, vec2(0.0, 0.0) + TexCoord0 + vec2(0, 1)/ sketchSize.xy).rgb;
	vec3 bottom = texture2D( sharpSampler, vec2(0.0, 0.0) + TexCoord0 + vec2(0, -1)/ sketchSize.xy).rgb;
	vec3 left = texture2D( sharpSampler, vec2(0.0, 0.0) + TexCoord0 + vec2(-1, 0)/ sketchSize.xy).rgb;
	vec3 right = texture2D( sharpSampler, vec2(0.0, 0.0) + TexCoord0 + vec2(1, 0)/ sketchSize.xy).rgb;
	vec3 color = texture2D( sharpSampler, TexCoord0 + vec2(0.0, 0.0)/sketchSize.xy).rgb;

	vec3 highColor = 4*color - top - bottom - left - right;
	color += lambda * highColor;

	gl_FragColor = vec4(color, 1.0);
}
