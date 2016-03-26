uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform float screen_width;
uniform float screen_height;

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

void main (void)
{
	float xstep = 1.0/screen_width;
	float ystep = 1.0/screen_height;
	float ratio = screen_width / screen_height;

	vec4 color = texture2D( Texture0 , vec2( gl_TexCoord[0] ) );
	float depth = texture2D( Texture1 , vec2( gl_TexCoord[0] ) ).r;

	//Permet de scaler la profondeur
	depth = LinearizeDepth(depth);

	// Kind of Sobel filter

	// to gray-scale
	vec4 grayColor = ToGray(color);

	// get neighbor pixels
	vec4 leftPixelGrayColor = ToGray(texture2D(Texture0, vec2(gl_TexCoord[0]) + vec2(-1, 0)));
	vec4 rightPixelGrayColor = ToGray(texture2D(Texture0, vec2(gl_TexCoord[0]) + vec2(1, 0)));
	vec4 topPixelGrayColor = ToGray(texture2D(Texture0, vec2(gl_TexCoord[0]) + vec2(0, -1)));
	vec4 bottomPixelGrayColor = ToGray(texture2D(Texture0, vec2(gl_TexCoord[0]) + vec2(0, 1)));

	vec4 topLeftPixelGrayColor = ToGray(texture2D(Texture0, vec2(gl_TexCoord[0]) + vec2(-1, -1)));
	vec4 topRightPixelGrayColor = ToGray(texture2D(Texture0, vec2(gl_TexCoord[0]) + vec2(1, -1)));
	vec4 bottomLeftPixelGrayColor = ToGray(texture2D(Texture0, vec2(gl_TexCoord[0]) + vec2(1, -1)));
	// vec4 bottomRightPixelGrayColor = ToGray(texture2D(Texture0, vec2(gl_TexCoord[0]) + vec2(1, -1)));
	vec4　bottomRightPixelGrayColor = ToGray(texture2D(Texture0, vec2(gl_TexCoord[0]) + vec2(1, 1)));
	// vec4　bottomRightPixelGrayColor = ToGray(texture2D(Texture0, vec2(gl_TexCoord[0]) + vec2(1, 1)));

	float gradientX = (rightPixelGrayColor - leftPixelGrayColor) / 2;
	float gradientY = (bottomPixelGrayColor - topPixelGrayColor) / 2;
	float gradientValue = sqrt(gradientX * gradientX + gradientY * gradientY);

	// if difference is above threshold, add black outline by returning black pixel
	// max is sqrt(2) * 255
	float normalizedDiff = gradientValue / sqrt(2) / 255.f;
	if (normalizedDiff > 0.8f)
		gl_FragColor = vec4(0, 0, 0, 1);
	else
		gl_FragColor = color;
}
