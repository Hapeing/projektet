struct Output
{
	float4 position_cs : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

Output VS_main(uint id: SV_VertexID)
{
	Output output;

	output.texcoord = float2((id << 1) & 2, id & 2);
	output.position_cs = float4(output.texcoord * float2(2, -2) + float2(-1, 1), 0, 1);

	return output;
}