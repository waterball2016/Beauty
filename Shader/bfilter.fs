#version 330

#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

#define SIGMA 10.0
#define BSIGMA 0.1
#define MSIZE 13

in vec2 TexCoord0;
uniform sampler2D bfSampler0;
uniform sampler2D bfSampler1;

uniform vec2 sketchSize;


float normpdf(in float x, in float sigma)
{
	return exp(-x*x/(sigma*sigma));
}

float normpdf3(in vec3 v, in float sigma)
{
	return exp(-dot(v,v)/(sigma*sigma));
}

void main(void)
{
	vec3 c = texture2D( bfSampler0, vec2(0.0, 0.0) + TexCoord0).rgb;
	float grad = texture2D( bfSampler1, vec2(0.0, 0.0) + TexCoord0).r;
	float smoother = 1;

	if (grad < 0.2 && c.r > 0.3725 && c.g > 0.1568 && c.b > 0.0784 && c.r > c.b && max(max(c.r, c.g), c.b) - min(min(c.r, c.g), c.b) > 0.0588 && abs(c.r - c.g) > 0.0588)
	{
		//declare stuff
		const int kSize = (MSIZE-1)/2;
		float kernel[MSIZE];
		vec3 final_colour = vec3(0.0);

		//create the 1-D kernel
		float Z = 0.0;
		for (int j = 0; j <= kSize; ++j)
		{
			kernel[kSize+j] = kernel[kSize-j] = normpdf(float(j), SIGMA);
		}


		vec3 cc;
		float factor;
		for (int i=-kSize; i <= kSize; ++i)
		{
			for (int j=-kSize; j <= kSize; ++j)
			{
				cc = texture2D(bfSampler0, vec2(0.0, 0.0) + TexCoord0 + vec2(float(i),float(j)) / sketchSize.xy ).rgb;
				factor = normpdf3(cc-c, BSIGMA) * kernel[kSize+i] * kernel[kSize+j];;	//Nicer one
				//factor = (0.18 + i*0.03) * (0.18 + i*0.03) * (1.0 - min(distance(cc, c), 1.0));		//Efficient one
				Z += factor;
				final_colour += factor*cc;
			}
		}

		final_colour = final_colour/Z;
		c = mix(c, final_colour, smoother);
	}
	gl_FragColor = vec4(c, 1.0);
}
