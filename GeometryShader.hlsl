struct VS_OUT
{
	float4 Pos : POSITION;
	float2 UV : TEXCOORD;
};

struct GS_OUT
{
	float4 Pos : SV_POSITION;
	float4 Nor : NORMAL;
	float2 UV : TEXCOORD;
};

cbuffer WVP : register(b0)
{
	float4x4 wvp_mat;
}

[maxvertexcount(3)]
void GS_main(
	triangle	VS_OUT input[3] : SV_POSITION,
	inout		TriangleStream< GS_OUT > output
)
{
	//Calculate normal
	float4 faceEdgeA  = input[1].Pos - input[0].Pos;
	float4 faceEdgeB  = input[2].Pos - input[0].Pos;
	float3 faceNormal = normalize(cross(faceEdgeA.xyz, faceEdgeB.xyz));


	//Input triangle, transformed
	for (uint i = 0; i < 3; i++)
	{
		GS_OUT element;
		element.Pos = mul(input[i].Pos, wvp_mat);
		element.Nor = mul(float4(faceNormal, 0.0f), wvp_mat);
		element.UV = input[i].UV;
		output.Append(element);
	}
	output.RestartStrip();
}