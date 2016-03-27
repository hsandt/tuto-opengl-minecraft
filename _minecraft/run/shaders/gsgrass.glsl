#version 400 core

uniform float elapsed;

layout (triangles) in;
layout (triangle_strip, max_vertices = 33) out;

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

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void drawBlade (vec4 bladeBottom) {
    float angle = 2 * 3.14f * ((bladeBottom.x + bladeBottom.y) / 10 - elapsed / 2.5);
    float topOffsetX = 0.8 * sin(angle);

    gl_Position = bladeBottom + vec4(0, 0, 0, 0); EmitVertex();
    gl_Position = bladeBottom + vec4(0.3, 0, 0, 0); EmitVertex();
    gl_Position = bladeBottom + vec4(0 + topOffsetX * 0.8, 2 + topOffsetX * 0.1, 0, 0); EmitVertex();
    gl_Position = bladeBottom + vec4(0.4 + topOffsetX * 0.8, 2, 0, 0); EmitVertex();
    gl_Position = bladeBottom + vec4(-0.5 + topOffsetX, 3.5, 0, 0); EmitVertex();
    EndPrimitive();
}

void main(void)
{
    vec3 ab = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 ac = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 normal = normalize(cross(ab, ac));

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
        EmitVertex();
    }
    EndPrimitive();

    // grass blade for upper surface
    // actually we receive vertices in screen coords (very hard to deal with, but nice for billboarding)
    // if (dot(normal, vec3(0, 0, 1)) > 0.5)
    if (dot(normal, vec3(0, -1, 0)) > 0.25)  // upper surface -> normal is +Z in world -> but -Y in screen space
    {
        // XY screen billboard (for some reason, here -Y is really screen bottom)
        // put blade on grass triangle barycenters (we average w coord too but since it represents depth, linear op is fine)
        vec4 bladeBottom0 = gl_in[0].gl_Position * 0.5 + gl_in[1].gl_Position * 0 + gl_in[2].gl_Position * 0.5;
        drawBlade(bladeBottom0);
        vec4 bladeBottom1 = gl_in[0].gl_Position * 0.45 + gl_in[1].gl_Position * 0.1 + gl_in[2].gl_Position * 0.45;
        drawBlade(bladeBottom1);
        vec4 bladeBottom2 = gl_in[0].gl_Position * 0.35 + gl_in[1].gl_Position * 0.3 + gl_in[2].gl_Position * 0.35;
        drawBlade(bladeBottom2);
        vec4 bladeBottom3 = gl_in[0].gl_Position * 0.2 + gl_in[1].gl_Position * 0.6 + gl_in[2].gl_Position * 0.2;
        drawBlade(bladeBottom3);
        vec4 bladeBottom4 = gl_in[0].gl_Position * 0.15 + gl_in[1].gl_Position * 0.7 + gl_in[2].gl_Position * 0.15;
        drawBlade(bladeBottom4);

        // for (int i = 0; i < 3; i++)
        // {
        //     gl_Position = gl_in[i].gl_Position; // + vec4(1 * normal, 0);
        //     EmitVertex();
        // }
        // EndPrimitive();
    }
}
