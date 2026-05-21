#include "PipelineState.h"
#include "KamataEngine.h"
#include <cassert>

using namespace KamataEngine;

// PipelineStateを生成する

void PipelineState::Create(D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc) {

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	HRESULT hr;

	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	pipelineState_ = graphicsPipelineState;
}

ID3D12PipelineState* PipelineState::Get() { return pipelineState_; }

PipelineState::PipelineState() {}

PipelineState::~PipelineState() {
	if (pipelineState_) {
		pipelineState_->Release();
		pipelineState_ = nullptr;
	}
}
