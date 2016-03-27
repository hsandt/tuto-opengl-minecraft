varying vec3 normal;
varying vec3 vertex_to_light_vector;
varying vec3 vertex_to_camera_vector;

uniform vec4 ambientColor;  // use alpha
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;

uniform float ambientLevel;
uniform float diffuseLevel;
uniform float specularLevel;

uniform sampler2D Texture0;

void main()
{
	// cannot make textures work!!
	// vec4 color = texture2D( Texture0 , vec2( gl_TexCoord[0] ) );

	// Scaling The Input Vector To Length 1
	vec3 normalized_normal = normalize(normal);
	vec3 normalized_vertex_to_light_vector = normalize(vertex_to_light_vector);
	vec3 normalized_vertex_to_camera_vector = normalize(vertex_to_camera_vector);

	// Calculating The Diffuse Term And Clamping It To [0;1]
	float DiffuseTerm = clamp(dot(normalized_normal, normalized_vertex_to_light_vector), 0.0, 1.0);

	// Calculating The Specular Term And Clamping It To [0;1]
	vec3 halfwayVector = (normalized_vertex_to_light_vector + normalized_vertex_to_camera_vector) / 2;
	halfwayVector = normalize(halfwayVector);
	float specularAngle = max(dot(normalized_normal, halfwayVector), 0.0);  // only positive values
	float SpecularTerm = pow(specularAngle, shininess);

	// Calculating The Final Color
	gl_FragColor.rgb = ambientColor.rgb * ambientLevel + diffuseColor * DiffuseTerm * diffuseLevel + specularColor * SpecularTerm * specularLevel;
	gl_FragColor.a = ambientColor.a;

	// gl_FragColor = color;
}
