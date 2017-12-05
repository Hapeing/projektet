//-------------------------------
//EXACTLY THE SAME AS FORWARD RENDERING VS
//-------------------------------

struct VS_IN
{
	float3 Pos : POSITION;
	float3 Nor : NORMAL;
	float2 UV : TEXCOORD;
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float4 PosW : POSITION;
	float4 NorW : NORMAL;
	float2 UV : TEXCOORD;
};

cbuffer WVP : register(b0)
{
	float4x4 wvp_mat; //World*View*Projection matrix
	float4x4 w_mat; //World projection matrix
	float4x4 it_w_mat; //Inverse transpose of world matrix (for normal)
}

//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	//multiply stuff
	output.Pos = mul(float4(input.Pos, 1.0f), wvp_mat);
	output.PosW = mul(float4(input.Pos, 1.0f), w_mat);
	output.NorW = mul(float4(input.Nor, 0.0), it_w_mat);
	output.UV = input.UV;

	return output;
}