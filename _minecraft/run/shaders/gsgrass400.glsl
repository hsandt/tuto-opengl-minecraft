#version 430 core

layout (triangles) in;
layout (triangle_strip) out;
layout (max_vertices = 3) out;

in VertexData
{
    vec3 normal;
    vec4 color;
} vertex[];

out FragData
{
    vec3 normal;
    vec4 color;
} gs_out[];

in vec3 normal[3];

// void make_face(vec3 a, vec3 b, vec3 c)
// {
//     vec3 face_normal = normalize(cross(c - a, c - b));
//     vec4 face_color = vec4(1.0, 0.2, 0.4, 1.0);
//     gl_Position = gl_ModelViewProjectionMatrix * vec4(a, 1.0);
//     color = face_color;
//     EmitVertex();

//     gl_Position = gl_ModelViewProjectionMatrix * vec4(b, 1.0);
//     color = face_color;
//     EmitVertex();

//     gl_Position = gl_ModelViewProjectionMatrix * vec4(c, 1.0);
//     color = face_color;
//     EmitVertex();

//     EndPrimitive();
// }

void main(void)
{
    // vec3 ab = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    // vec3 ac = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    // vec3 normal = normalize(cross(ab, ac));

    // // Copy the incoming vertex positions into some local variables
    // vec3 a = gl_in[0].gl_Position.xyz;
    // vec3 b = gl_in[1].gl_Position.xyz;
    // vec3 c = gl_in[2].gl_Position.xyz;

    // // Find a scaled version of their midpoints
    // vec3 d = (a + b) * stretch;
    // vec3 e = (b + c) * stretch;
    // vec3 f = (c + a) * stretch;

    // // Now, scale the original vertices by an inverse of the midpoint
    // // scale
    // a *= (2.0 - stretch);
    // b *= (2.0 - stretch);
    // c *= (2.0 - stretch);

    // make_face(a, d, f);
    // make_face(d, b, e);
    // make_face(e, c, f);
    // make_face(d, e, f);

    for (int i = 0; i < 3; i++)
    {
        gl_Position = gl_in[i].gl_Position; // + vec4(1 * normal, 0);
        gs_out[i].normal = vertex.normal[i];
        // frag.normal = vertices[i].normal;
        // frag.color = vertices[i].color;
        EmitVertex();
    }
    EndPrimitive();
}
