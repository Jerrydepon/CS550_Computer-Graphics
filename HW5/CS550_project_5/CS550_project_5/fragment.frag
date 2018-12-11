#version 120 // not compatible with 330

uniform float uKa, uKd, uKs;    // coefficients of each type of lighting
uniform vec3 uColor;    // object color
uniform vec3 uSpecularColor;    // light color
uniform float uShininess;   // specular exponent

uniform float uS0, uT0;
uniform float Ds, Dt;
uniform float uSize;

varying vec2 vST;   // texture cords
varying vec3 vN;    // normal vector
varying vec3 vL;    // vector from point to light
varying vec3 vE;    // vector from point to eye
varying vec3 vColor;

uniform float uTime;
const float PI = 3.14159265;

uniform bool fragPattern;

void main( )
{
    vec3 Normal = normalize(vN);
    vec3 Light = normalize(vL);
    vec3 Eye = normalize(vE);
    
    vec3 myColor = uColor;
    if(fragPattern)
    {
        // pattern
//        if( uS0-uSize/2. <= vST.s && vST.s <= uS0+uSize/2. && uT0-uSize/2. <= vST.t && vST.t <= uT0+uSize/2. )
//        {
//            myColor = vec3( 0.4*sin(uTime * 10), 0.5*sin(uTime * 10), 0.6 );
//        }
        if ( (uS0*cos(Ds)+uT0*sin(Dt))-uSize/2. <= vST.s+vST.t
            && vST.s+vST.t <= (uS0*cos(Ds)+uT0*sin(Dt))+uSize/2.)
        {
            myColor = vec3( 0.4*sin(uTime * 20), 0.5*sin(uTime * 10), 0.6 );
        }
    }
    
    // light
    float d = max(dot(Normal, Light), 0. ); // only do diffuse if the light can see the point
    vec3 diffuse = uKd * d * uColor;
    
    float s = 0.;
    if( dot(Normal, Light) > 0. ) // only do specular if the light can see the point
    {
        vec3 ref = normalize( reflect( -Light, Normal ) );
        s = pow(max(dot(Eye, ref),0. ), uShininess);
        
    }
    vec3 ambient = uKa * myColor * 2;
    vec3 specular = uKs * s * uSpecularColor * 3;
    gl_FragColor = vec4( ambient + diffuse + specular, 1. );
//    gl_FragColor = vec4(vColor, 1.);
}

