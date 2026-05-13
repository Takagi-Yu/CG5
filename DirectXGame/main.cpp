#include "KamataEngine.h"
#include <Windows.h>

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	KamataEngine::Initialize(L"3161_ゲームタイトル");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		dxCommon->PreDraw();

		dxCommon->PostDraw();
	}

	KamataEngine::Finalize();

	return 0;
}