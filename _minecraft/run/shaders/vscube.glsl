varying vec3 normal;
varying vec3 vertex_to_light_vector;
varying vec3 vertex_to_camera_vector;

uniform float elapsed;
uniform mat4 invertView;

uniform float wave_amplitude;
uniform float normalized_wavelength;
uniform float wave_period;

attribute float wave_factor;  // 0 for no wave

void main()
{
	// Transforming The Vertex
	mat4 modelMatrix = invertView * gl_ModelViewMatrix;
	mat4 viewProjectionMatrix = gl_ModelViewProjectionMatrix * inverse(modelMatrix);
	vec4 worldVertex = modelMatrix * gl_Vertex;
	// 2pi*(x/L - t/T)
	float angle = 2 * 3.14f * ((worldVertex.x / 10.f) / normalized_wavelength - elapsed / wave_period);
	worldVertex.z += wave_factor * wave_amplitude * sin(angle);
	gl_Position = viewProjectionMatrix * worldVertex;
	// z = A*sin(a*x+b)
	// derivative is z' = A*a*cos(a*x+b)
	// positive tangent is (1, z') = (1, A*a*cos(a*x+b))
	// positive normal is (-z', 1) = (-A*a*cos(a*x+b), 1) where
	// A = wave_factor * wave_amplitude
	// a = 2 * 3.14f / 10.f / normalized_wavelength
	// b = - 2 * 3.14f * elapsed / wave_period
	// in 3D, it gives: (-z', 0, 1)
	// either we normalize this to get directly the normal, or we apply a rotation to the known normal
	// since our formula only works for a plane surface, we can just apply the normal formula

	// compute world normal manually IF Z+ surface! (test gl_Vertex.z or wave factor)
	if (wave_factor != 0)
	{
		float A = wave_factor * wave_amplitude;
		float a = 2 * 3.14f / 10.f / normalized_wavelength;
		float b = - 2 * 3.14f * elapsed / wave_period;
		normal = vec3(- A * a * cos(a * worldVertex.x + b), 0, 1);
	}
	else
	{
		normal = gl_Normal;
	}

	// for now, keep world normal... will se if need to convert to view coords

	// Transforming The Normal To ModelView-Space
	// ?? model matrix with rotation only should be enough?? if using view coord for normal,
	// light direction should also be or the dot product does not make sense!!
	// normal = gl_NormalMatrix * gl_Normal;

	// normal = invertView * normal;  // and without translation... or dot product will be wrong

	//Direction lumiere
	// !! directional light only, subtract by vertex position for point light
	vertex_to_light_vector = vec3(gl_LightSource[0].position);

	// Direction camera (non normalisee): camera transformation matrix * this vertex = invertView * this vertex = invertView translation part applied to this vertex
	vertex_to_camera_vector = vec3(invertView * worldVertex);

	//Couleur
	// color = gl_Color;
}
