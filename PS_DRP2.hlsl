Texture2D NormalTexture   : register(t0);
Texture2D DiffuseTexture  : register(t1);
Texture2D SpecularTexture : register(t2);
Texture2D PositionTexture : register(t3);

//Hard coded lights
struct PLight
{
	float4 lightPosition;
	float4 lightColor;	  //.a is intensity
};

float lights[16] = 
{
	50.0, 100.0, -30.0, 1.0, //pos
	1.0, 1.0, 1.0, 1.0,	//greyish color

	150, 100.0, 30.0, 1.0, //pos
	0.0, 1.0, 1.0, 1.0	//greyish color
};

void GetAttributes
(
	in float2 screenPosition,
	out float3 normal,
	out float3 position,
	out float3 diffuse,
	out float3 specular,
	out float specularPower
)
{
	int3 sampledIndex = int3(screenPosition, 0);

	normal = NormalTexture.Load(sampledIndex.xyz).xyz;
	position = PositionTexture.Load(sampledIndex.xyz).xyz;
	diffuse = DiffuseTexture.Load(sampledIndex.xyz).xyz;
	float4 spec = SpecularTexture.Load(sampledIndex.xyz);
	
	specular = spec.xyz;
	specularPower = spec.w;
}

float ComputeDiffuseFactor(float3 lightVector, float3 normal)
{
	return dot(lightVector, normal);
}

cbuffer PointLight : register(b0)
{
	float4 lightPosition;
	float4 lightColor;	  //.a is intensity
}

cbuffer Camera : register(b1)
{
	float4 CamPos; //in world (ignore .w)
}

float4 PS_main(in float4 screenPos : SV_Position) : SV_TARGET0
{

	float3 NorW;
	float3 PosW;
	float3 diffuse;
	float3 specular;
	float specularPower;

	GetAttributes(screenPos.xy, NorW, PosW, diffuse, specular, specularPower);
	//normalize normal
	NorW.xyz = normalize(NorW.xyz);

	float4 final = float4(0.0, 0.0, 0.0, 1.0);
	for (int i = 0; i < 1; i++)
	{
		//float3 lightPos = float3(lights[0 + i * 8] , lights[1 + i * 8], lights[2 + i * 8]);
		//float4 lightCol = float4(lights[4 + i * 8], lights[5 + i * 8], lights[6 + i * 8], lights[7 + i * 8]);

		float3 lightPos = lightPosition.xyz;
		float4 lightCol = lightColor;
		//normalized vector from fragment in world space to light
		float3 lightVector = normalize(lightPos - PosW.xyz);

		//---------------------------
		//             Specular
		//---------------------------

		float3 N = NorW.xyz;
		float3 L = normalize(lightPos - PosW);
		float3 V = normalize(CamPos.xyz - PosW.xyz);
		float3 R = 2 * dot(N, L) * N - L;


		//float3 R = normalize((2 * dot(input.NorW.xyz, lightVector) * input.NorW.xyz) - lightVector);

		float specularFactor = pow(max(dot(V, R), 0.0f), specularPower);

		//---------------------------

		//cos(angle) between light vector and normal of fragment (since normalized this is just dot prod)
		float diffuseFactor = max(ComputeDiffuseFactor(lightVector, NorW.xyz), 0.0f);


		//initialize final with ambient as ambientColor*lightcol*fragmentCol*0.1
		float3 ambientColor = diffuse * float3(0.2, 0.2, 0.2); //Never pitch black color
		float4 thisColor = float4(ambientColor*lightCol*diffuse * 0.3f, 1.0f);

		//calculate color from this light
		thisColor =
			float4((diffuseFactor * diffuse * lightCol.xyz	+ (specular * specularFactor)), 1.0f); // Diffuse factor(no attenuation)				//Specular factor

		final += float4(thisColor.xyz, 1.0);
	}


	//clamp to 0.0-1.0 and return final
	return min(final, float4(1.0, 1.0, 1.0, 1.0));
};