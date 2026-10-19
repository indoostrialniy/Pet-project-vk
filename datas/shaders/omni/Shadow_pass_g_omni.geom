#version 450
#extension GL_NV_viewport_array2 : require  // опционально, для multiviewport

layout(triangles, invocations = 6) in;
layout(triangle_strip, max_vertices = 3) out;


mat4 lookAtRH(vec3 eye, vec3 center, vec3 up)
{
	vec3 f = normalize(center - eye);
	vec3 s = normalize(cross(f, up));
	vec3 u = cross(s, f);

	mat4 Result = mat4(1.0);
		Result[0][0] = s.x;
		Result[1][0] = s.y;
		Result[2][0] = s.z;
		Result[0][1] = u.x;
		Result[1][1] = u.y;
		Result[2][1] = u.z;
		Result[0][2] =-f.x;
		Result[1][2] =-f.y;
		Result[2][2] =-f.z;
		Result[3][0] =-dot(s, eye);
		Result[3][1] =-dot(u, eye);
		Result[3][2] = dot(f, eye);

	return Result;
	//return transpose(Result);
}


mat4 perspectiveRH_NO(float fovy, float aspect, float zNear, float zFar)
{
	//assert(abs(aspect - std::numeric_limits<T>::epsilon()) > static_cast<T>(0));

	float tanHalfFovy = tan(fovy / 2);

	mat4 Result = mat4(0.0);
		Result[0][0] = 1 / (aspect * tanHalfFovy);
		Result[1][1] = 1 / (tanHalfFovy);
		Result[2][2] = - (zFar + zNear) / (zFar - zNear);
		Result[2][3] = - 1;
		Result[3][2] = - (2 * zFar * zNear) / (zFar - zNear);
	
	//return transpose(Result);
	return Result;
}

layout(binding = 0) uniform FhShadowpassCubeEdgesInfo  // set = 0, 
{
    mat4 PV[6];
};

layout(location = 0) out vec4 FragPos;

in gl_PerVertex {
    vec4 gl_Position;
} gl_in[];

out gl_PerVertex {
    vec4 gl_Position;
};

void main() 
{
    gl_Layer = gl_InvocationID;   // ← НАПРАВЛЯЕМ В КОНКРЕТНУЮ ГРАНЬ

	//vec3 lightPos = vec3(6.97, 8.97, 2.96);

	//mat4 proj = perspectiveRH_NO(90*3.14/180, 1, 0.2, 40); 

	//mat4 PV2[6];
	//	PV2[0] = proj * lookAtRH( lightPos, lightPos + vec3(1,0,0), vec3(0,-1,0) );
	//	PV2[1] = proj * lookAtRH( lightPos, lightPos + vec3(-1,0,0), vec3(0,-1,0) );
	//	PV2[2] = proj * lookAtRH( lightPos, lightPos + vec3(0,1,0), vec3(0,0,1) );
	//	PV2[3] = proj * lookAtRH( lightPos, lightPos + vec3(0,-1,0), vec3(0,0,-1) );
	//	PV2[4] = proj * lookAtRH( lightPos, lightPos + vec3(0,0,1), vec3(0,-1,0) );
	//	PV2[5] = proj * lookAtRH( lightPos, lightPos + vec3(0,0,1), vec3(0,-1,0) );

    for (int i = 0; i < 3; i++) {
        gl_Position = PV[gl_InvocationID] * gl_in[i].gl_Position; // non-linear!
		FragPos = gl_in[i].gl_Position;
		//gl_Position.z = pow(gl_Position.z, 10.5);
		//gl_Position.z /= gl_Position.w;
        EmitVertex();
    }
    EndPrimitive();
}
