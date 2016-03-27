uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform float screen_width;
uniform float screen_height;

uniform float outline_threshold;

uniform float temp;

float LinearizeDepth(float z)
{
	float n = 0.5; // camera z near
  	float f = 10000.0; // camera z far
  	return (2.0 * n) / (f + n - z * (f - n));
}

vec4 ToGray(vec4 color)
{
	return vec4(dot(vec3(1, 1, 1), color.rgb) / 3 * vec3(1, 1, 1), color.a);
}

vec4 Blur(vec4 color, ivec2 screenPixelCoords, int blurRadius)
{
	// this blur uses the passed color which may have received some post-process,
	// but other pixels are not processed!
	vec4 colorSum = color;
	for (int i = -blurRadius; i <= blurRadius; ++i)
		for (int j = -blurRadius; j <= blurRadius; ++j)
		{
			if (i == 0 && j == 0) continue;
			colorSum += texelFetch(Texture0, screenPixelCoords + ivec2(i, j), 0);
		}
	int blurSize = 2 * blurRadius + 1;
	color = colorSum / (blurSize * blurSize);
	return color;
}

void main (void)
{
	float xstep = 1.0/screen_width;
	float ystep = 1.0/screen_height;
	float ratio = screen_width / screen_height;

	ivec2 screenPixelCoords = ivec2(gl_TexCoord[0].x * screen_width, gl_TexCoord[0].y * screen_height);

	// vec4 color = texture2D( Texture0 , vec2( gl_TexCoord[0] ) );
	vec4 color = texelFetch( Texture0, screenPixelCoords, 0 );
	// float depth = texture2D( Texture1 , vec2( gl_TexCoord[0] ) ).r;
	float depth = texelFetch( Texture1, screenPixelCoords, 0 );

	//Permet de scaler la profondeur
	depth = LinearizeDepth(depth);  // between 1e-8 and 1/5000



	// OUTLINE

	// to gray-scale
	vec4 grayColor = ToGray(color);

	vec4 leftPixelGrayColor = ToGray(texelFetch(Texture0, screenPixelCoords + ivec2(-1, 0), 0));
	vec4 rightPixelGrayColor = ToGray(texelFetch(Texture0, screenPixelCoords + ivec2(1, 0), 0));
	vec4 topPixelGrayColor = ToGray(texelFetch(Texture0, screenPixelCoords + ivec2(0, -1), 0));
	vec4 bottomPixelGrayColor = ToGray(texelFetch(Texture0, screenPixelCoords + ivec2(0, 1), 0));

	vec4 topLeftPixelGrayColor = ToGray(texelFetch(Texture0, screenPixelCoords + ivec2(-1, -1), 0));
	vec4 topRightPixelGrayColor = ToGray(texelFetch(Texture0, screenPixelCoords + ivec2(1, -1), 0));
	vec4 bottomLeftPixelGrayColor = ToGray(texelFetch(Texture0, screenPixelCoords + ivec2(1, -1), 0));
	vec4 bottomRightPixelGrayColor = ToGray(texelFetch(Texture0, screenPixelCoords + ivec2(1, 1), 0));

	float gradientX = 0.25 * ((rightPixelGrayColor - leftPixelGrayColor) * 2 + (topRightPixelGrayColor - topLeftPixelGrayColor) + (bottomRightPixelGrayColor - bottomLeftPixelGrayColor));
	float gradientY = 0.25 * ((bottomPixelGrayColor - topPixelGrayColor) * 2 + (bottomLeftPixelGrayColor - topLeftPixelGrayColor) + (bottomRightPixelGrayColor - topRightPixelGrayColor));
	float gradientValue = sqrt(gradientX * gradientX + gradientY * gradientY);

	// if difference is above threshold, add black outline by returning black pixel
	if (gradientValue * 1000 > outline_threshold)
		color = vec4(0, 0, 0, 1);

	// debug Sobel filter
	// color = vec4(vec3(1, 1, 1) * gradientValue, 1);


	// WARNING: to really chain post-process shaders you must call 2 different shaders in the program
	// otherwise, combination may cause overriding of some processing
	// ex: Outline will be computed for current pixel but we do not know outline for pixels around


	// BLUR

	//beyond some depth (depth of field)

	// DEBUG
	// color = vec4(vec3(0, 0, 1) * depth * 15, 1);

	if (depth * 15 > 1.0)
	{
		// apply blur (BLUR_SIZExBLUR_SIZE-square mean)
		// the farther, the blurrier
		// DEBUG
		// color = vec4(1,1,1,1);
		int blurRadius = (int) depth * 15;
		color = Blur(color, screenPixelCoords, blurRadius);
	}



	gl_FragColor = color;
}
