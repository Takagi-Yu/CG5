#include "VertexBuffer.h"
#include "KamataEngine.h"

#include <d3d12.h>
#include <cassert>

using namespace KamataEngine;

void VertexBuffer::Create(const UINT size, const UINT stride) { 
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	HRESULT hr;

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
	hr =
	    dxCommon->GetDevice()->CreateCommittedResource(&uploadProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	vertexBuffer_ = vertexResource;

	// vertexBufferViewを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭アドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点sizeのサイズ
	vertexBufferView.SizeInBytes = size;
	// 1つの頂点のサイズ
	vertexBufferView.StrideInBytes = stride;

	vertexBufferView_ = vertexBufferView;
}

ID3D12Resource* VertexBuffer::Get() { return vertexBuffer_; }

D3D12_VERTEX_BUFFER_VIEW* VertexBuffer::GetView() { return &vertexBufferView_; }

VertexBuffer::VertexBuffer() {}

VertexBuffer::~VertexBuffer() {
	if (vertexBuffer_) {
		vertexBuffer_->Release();
		vertexBuffer_ = nullptr;
	}
}
