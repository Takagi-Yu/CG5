#include "KamataEngine.h"
#include "Shader.h"
#include "RootSignature.h"
#include <Windows.h>
#include <cassert>

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	KamataEngine::Initialize(L"CG5");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf(
		std::format("width: {}, height: {}\n", w, h).c_str());

	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	RootSignature rs;
	rs.Create();

	//// RootSignature作成
	//// 構造体にデータを用意する
	//D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	//descriptionRootSignature.Flags = 
	//	D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//ID3DBlob* signatureBlob = nullptr;
	//ID3DBlob* errorBlog = nullptr;
	//HRESULT hr = D3D12SerializeRootSignature(
	//	&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlog);
	//if (FAILED(hr)) {
	//	DebugText::GetInstance()->ConsolePrintf(
	//		reinterpret_cast<char*>(errorBlog->GetBufferPointer()));
	//	assert(false);
	//}

	//// バイナリをもとに生成
	//ID3D12RootSignature* rootSignature = nullptr;
	//hr = dxCommon->GetDevice()->CreateRootSignature(
	//	0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	//assert(SUCCEEDED(hr));

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendState
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RansterrizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面をカリングする
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 塗りつぶしモードをソリッドにする(ワイヤーフレームならD3D12_FILL_MODE_WIREFRAME)
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	// コンパイル済みのShader、エラー時情報の格納場所の用意


	Shader vs;
	vs.LoadDxc(L"Resources/shaders/TestVS.hlsl", L"vs_6_0");
	assert(vs.GetDxcBlob() != nullptr);


	Shader ps;
	ps.LoadDxc(L"Resources/shaders/TestPS.hlsl", L"ps_6_0");
	assert(ps.GetDxcBlob() != nullptr);


	// PSO(PipelineStateObject)の生成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rs.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = {vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()};
	graphicsPipelineStateDesc.PS = {ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()};
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	// 書き込むRTVの情報 
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// PSOを生成する
	ID3D12PipelineState* graphicsPipelineState = nullptr;
	HRESULT hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	// VertexResourceの生成
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadProperties{};
	uploadProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUから書き込むヒープ
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeof(Vector4) * 3;
	// バッファの場合はこれらを1にする
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを生成する
	ID3D12Resource* vertexResource = nullptr;
	hr = dxCommon->GetDevice()->CreateCommittedResource(
		&uploadProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, 
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	// vertexBufferViewを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭アドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(Vector4) * 3;
	// 1つの頂点のサイズ
	vertexBufferView.StrideInBytes = sizeof(Vector4);

	// 頂点リソースにデータを書き込む
	Vector4* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	vertexData[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
	vertexData[1] = {0.0f, 0.5f, 0.0f, 1.0f};
	vertexData[2] = {0.5f, -0.5f, 0.0f, 1.0f};
	// 頂点リソースのマップを解除する
	vertexResource->Unmap(0, nullptr);






	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		dxCommon->PreDraw();

		// コマンドを積む
		commandList->SetGraphicsRootSignature(rs.Get());
		commandList->SetPipelineState(graphicsPipelineState);
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		// トポロジの設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 緒点数、インデックス数、インデックスの開始位置、インデックスのオフセット
		commandList->DrawInstanced(3, 1, 0, 0);






		dxCommon->PostDraw();
	}

	vertexResource->Release();
	graphicsPipelineState->Release();
	//signatureBlob->Release();

	//rootSignature->Release();


	KamataEngine::Finalize();

	return 0;
}