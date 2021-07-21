#version 330 core
out vec4 fragColor ;

float LinearizeDepth(float depth);
in vec2 TextCoords;

uniform sampler2D scene;
uniform sampler2D depthMap;

const float near_plane = 1;
const float far_plane = 100;

void main()
{	
	//vec4 orig = texture2D(scene, TextCoords);
	//fragColor = 1 - orig; // inverse colour
	//
	//float avg = (orig.x + orig.y + orig.z)/3;
	//fragColor = vec4(vec3(avg),1.0); // gray scale

	float depth = texture(depthMap, TextCoords).r;
	fragColor = vec4(vec3(LinearizeDepth(depth)/ far_plane), 1.0);
	
	fragColor = vec4(vec3(depth), 1.0);
}	

float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z *(far_plane - near_plane)); 
}
