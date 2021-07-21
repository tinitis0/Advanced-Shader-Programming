
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTextCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;  // this is different - we're passing this to calculate shadow in frag shader

out vec2 textCoord;
out vec3 fragPos;
out vec4 FragPosLightSpace;  // pass it on as a vec4

void main()
{
    textCoord = aTextCoord;  
	gl_Position = projection * view * model * vec4(aPos, 1.0);  // point as camera sees it
    fragPos = vec3(model * vec4(aPos, 1.0));  
	FragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);  // point as light sees it    
}