#version 450
// к binding = 0 будем ссылаться в descriptor layout
layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

//push constants block
layout( push_constant ) uniform constants
{
	vec4 data;
	mat4 model;
} PushConstants;


void main() 
{
	// PushConstants.data
	gl_Position = ubo.proj * ubo.view * PushConstants.model * vec4(inPosition*PushConstants.data.x, 1.0);
    fragColor = inColor;
	fragTexCoord = inTexCoord;
}