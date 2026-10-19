#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in uint inMaterialIndex;
layout(location = 4) in vec3 inTangent;

//push constants block
layout( push_constant ) uniform FhShadowpassPushConstantsInfo
{
	mat4 model;	
	mat4 PV;
} PushConstants;


void main() 
{
	// with geometry shader
	gl_Position = PushConstants.model * vec4(inPosition, 1.0);
}