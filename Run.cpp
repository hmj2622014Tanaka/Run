#include "DxLib.h"

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	// 定数
	const int WIDTH = 1600, HEIGHT = 1200;

	SetWindowText("Runゲーム"); //ウィンドウのタイトル
	SetGraphMode(WIDTH, HEIGHT, 32); //ウィンドウの大きさとカラービット数の指定
	ChangeWindowMode(TRUE); //ウィンドウモードで起動
	if (DxLib_Init() == -1) return -1; //ライブラリ初期化　エラーが起きたら終了
	SetBackgroundColor(0, 0, 0); //背景色の指定
	SetDrawScreen(DX_SCREEN_BACK); //描画面を裏画面にする

	int bgx = 0;
	int imgBG = LoadGraph("image/bg.jpg");

	while (1)
	{
		ClearDrawScreen();

		// 背景のスクロール処理
		bgx = bgx - 5;
		if (bgx <= -WIDTH)
		{
			bgx = 0;
		}
		DrawGraph(bgx, 0, imgBG, false);
		DrawGraph(bgx + WIDTH, 0, imgBG, false);

		ScreenFlip(); //裏画面の内容を表画面に反映させる
		WaitTimer(16); //一定時間待つ
		if (ProcessMessage() == -1) break;
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) break;
	}

	DxLib_End();
	return 0;
}