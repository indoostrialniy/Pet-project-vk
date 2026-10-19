#version 450
layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D maps[2];

vec4 getBloom()
{
	return texture(maps[1], fragTexCoord);
	
	int border = 3;
	int step1 = 1;

	float texelSize = 1.0/768.0;

	int count = 1;
	vec3 color = vec3(0);

	for(int i=-border; i<= border; i+=step1)
	{
		for(int j=-border; j<= border; j+=step1)
		{
			//if( length(texture(maps[1], fragTexCoord).rgb) > 0.01) 
			{
				color += texture(maps[1], fragTexCoord + vec2(i,j)*texelSize).rgb;
			}
			count++;
		}
	}
	color /= count;
	
	return vec4(color,1);
}


void main() {
    outColor = vec4(1,1,0,1);	//
	outColor = texture(maps[0], fragTexCoord) + getBloom();
	//outColor += outColor * texture(maps[1], fragTexCoord);
}