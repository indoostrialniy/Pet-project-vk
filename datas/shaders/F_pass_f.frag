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


vec3 ACESFilm(vec3 x) // Narcowicz
{
    // Нужен линейный HDR-вход. a и b подобраны так,
    // что кривая проходит через (0,0) и (1,1)
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

const mat3 ACESInputMat = mat3(
    0.59719, 0.35458, 0.04823,
    0.07600, 0.90834, 0.01566,
    0.02840, 0.13383, 0.83777
);

const mat3 ACESOutputMat = mat3(
     1.60475, -0.53108, -0.07367,
    -0.10208,  1.10813, -0.00605,
    -0.00327, -0.07276,  1.07602
);

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 ACESFitted(vec3 color)
{
    color = ACESInputMat * color;
    // RRT -> ODT в одном проходе
    color = RRTAndODTFit(color);
    color = ACESOutputMat * color;
    return clamp(color, 0.0, 1.0);
}

vec3 srgb_to_linear(vec3 c) 
{
    return mix(pow((c + 0.055) / 1.055, vec3(2.4)) * 1.055 - 0.055,
               c / 12.92,
               lessThanEqual(c, vec3(0.04045)));
}

vec3 linear_to_srgb(vec3 c) 
{
	vec3 low = c * 12.92;
	vec3 high = 1.055 * pow(c, vec3(1.0/2.4)) - 0.055;

    return mix(high, low, step(c, vec3(0.0031308)));
               
}

void main() {
	//outColor = texture(maps[0], fragTexCoord);// put linear color to exit as is
	//return;


	vec3 hdrColor = texture(maps[0], fragTexCoord).rgb; // linear

	//hdrColor = vec3(0.5);

	//float gamma = 2.2;
	float exposure = 0.0;

	hdrColor *= exp2(exposure);                      // экспозиция

    // тональная компрессия с экспозицией
    //vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
	//vec3 mapped = ACESFilm(hdrColor);
	//mapped = ACESFitted(hdrColor);
	//mapped = hdrColor;
    // гамма-коррекция
    //mapped = pow(mapped, vec3(gamma));//	mapped = srgb_to_linear(mapped);	//mapped = linear_to_srgb(mapped);
	

	// — и вот теперь color grading, в display-referred [0..1] —
	//float luma = dot(mapped, vec3(0.2126, 0.7152, 0.0722));
	//mapped = mix(vec3(luma), mapped, 1.15);          // насыщенность +15%
	//mapped = (mapped - 0.5) * 1.08 + 0.5;            // контраст +8%
	// опционально: mapped = clamp(1.05 * mapped - 0.02, 0.0, 1.0); // лёгкий lift/gamma


	//mapped = pow(mapped, vec3(gamma));
    //mapped = srgb_to_linear(mapped);	

	outColor = vec4(hdrColor, 1);

	//outColor += outColor * texture(maps[1], fragTexCoord);
}