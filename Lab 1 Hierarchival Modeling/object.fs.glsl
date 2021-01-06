#version 430 core

in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

uniform bool useTexture;
uniform sampler2D textureSampler;

out vec4 color;

void main(){

	// Light emission properties
	vec3 LightColor = vec3(1,1,1);
	float LightPower = 50.0f;
	
	// Material properties
	vec3 MaterialDiffuseColor = vec3(0.5,0.5,1.0);
	if(useTexture)
	{
		MaterialDiffuseColor=texture( textureSampler, UV ).rgb;
	};
	vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

	float distance = length( vec3(7,7,10) - Position_worldspace );

	vec3 n = normalize( Normal_cameraspace );
	vec3 l = normalize( LightDirection_cameraspace );
	float cosTheta = clamp( dot( n,l ), 0,1 );
	
	vec3 E = normalize(EyeDirection_cameraspace);
	vec3 R = reflect(-l,n);
	float cosAlpha = clamp( dot( E,R ), 0,1 );
	
	color.rgb=// Ambient : simulates indirect lighting
		MaterialAmbientColor +
		// Diffuse 
		MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
		// Specular 
		MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);
	color.a=1;
}