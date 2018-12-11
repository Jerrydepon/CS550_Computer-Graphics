#version 120 // not compatible with 330

varying vec2 vST;   // texture coords
varying vec3 vN;    // normal vector
varying vec3 vL;    // vector from point to light
varying vec3 vE;    // vector from point to eye
//varying vec3 vColor;
uniform float uTime;
const float PI = 3.14159265;

uniform bool vertPattern;

vec3 LightPosition = vec3( 10, 20, 5. );

void main( )
{
    // pattern
    vec3 vert = gl_Vertex.xyz;
    if(vertPattern)
    {
        vert.x = vert.x * sin( uTime * 10);
        vert.y = vert.y * cos( uTime * 10);
    }
    
    // light
    vST = gl_MultiTexCoord0.st;
    vec4 ECposition = gl_ModelViewMatrix * gl_Vertex;
    vN = normalize( gl_NormalMatrix * gl_Normal );  // normal vector
    vL = LightPosition - ECposition.xyz;            // vector from the point to the light position
    vE = vec3( 0., 0., 0. ) - ECposition.xyz;       // vector from the point to the eye position
    
    gl_Position = gl_ModelViewProjectionMatrix * vec4(vert, 1.);
    
//    vec4 pos = gl_Vertex;
//    vColor = pos.xyz; // set rgb from xyz!
//    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

