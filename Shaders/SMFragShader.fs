#version 330 core
out vec4 FragColor ;

uniform sampler2D SM;
in vec2 TextCoords;
in vec3 Pos;

void main()
{	
	FragColor = texture2D(SM,TextCoords);

	
}	
