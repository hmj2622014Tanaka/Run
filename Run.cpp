#include "DxLib.h"

// 2つの四角形が当たっているかを判定する関数
bool CheckCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
	if (x1 < x2 + w2 && x1 + w1 > x2 &&
		y1 < y2 + h2 && y1 + h1 > y2)
	{
		return true;
	}
	return false;
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	// 定数
	const int WIDTH = 1600, HEIGHT = 1200;

	SetWindowText("Runゲーム");
	SetGraphMode(WIDTH, HEIGHT, 32);
	ChangeWindowMode(TRUE);
	if (DxLib_Init() == -1) return -1;
	SRand(GetNowCount());
	SetBackgroundColor(0, 0, 0);
	SetDrawScreen(DX_SCREEN_BACK);

	int bgx = 0;
	int imgBG = LoadGraph("image/bg.jpg");

	//音ファイルの読み込み
	int bgmTitle = LoadSoundMem("sound/title.mp3");
	int bgmGame = LoadSoundMem("sound/bgm.mp3");
	int bgmGameOver = LoadSoundMem("sound/gameover.mp3");
	int seHit = LoadSoundMem("sound/hit.mp3");

	int score = 0;
	int highScore = 0;

	const int GROUND_Y = 1000;
	int playerWidth = 120;
	int playerHeight = 160;

	// プレイヤーの変数
	int playerX, playerY, PlayerVY;
	bool isJumping, isRight;

	const int MOVE_SPEED = 8;
	const int JUMP_POWER = -20;
	const int GRAVITY = 1;

	// アイテムの変数
	int itemStock = 0;
	int invincibleTimer = 0;
	const int INVINCIBLE_DURATION = 300;

	struct Item {
		int x, y;
		int width, height;
		bool visible;
	};
	Item starItem;
	int imgItem = LoadGraph("image/star.png");

	enum TogeType
	{
		TOGE_1 = 0,
		TOGE_2,
		TOGE_3,
		TOGE_4,
		TOGE_MAX
	};

	int togeWidths[TOGE_MAX] = { 100, 110, 80, 60 };
	int togeHeights[TOGE_MAX] = { 80, 120, 90, 60 };

	int imgToge[TOGE_MAX];
	imgToge[TOGE_1] = LoadGraph("image/spikeA.png");
	imgToge[TOGE_2] = LoadGraph("image/spikeB.png");
	imgToge[TOGE_3] = LoadGraph("image/spikeC.png");
	imgToge[TOGE_4] = LoadGraph("image/spikeD.png");

	struct Toge
	{
		int x, y;
		int vy;
		bool isLaunched;
		TogeType type;
		bool visible;
		bool isChecked;
	};

	const int MAX_TOGE = 5;
	Toge toges[MAX_TOGE];

	int togeTimer;
	const int TOGE_APPEAR_INTERVAL = 120;

	enum SceneType
	{
		SCENE_TITLE,    // タイトル画面
		SCENE_GAME,     // ゲーム本編
		SCENE_GAMEOVER  // ゲームオーバー画面
	};
	SceneType Scene = SCENE_TITLE; // 起動時はタイトル画面にする

	bool isReversed = false;

	// ゲームの状態を初期化するラムダ関数
	auto InitGame = [&]() {
		bgx = 0;
		playerX = 200;
		playerY = GROUND_Y - playerHeight;
		PlayerVY = 0;
		isJumping = false;
		isRight = true;

		isReversed = false;

		togeTimer = 0;
		for (int i = 0; i < MAX_TOGE; ++i)
		{
			toges[i].visible = false;
		}

		itemStock = 0;
		invincibleTimer = 0;
		starItem.width = 60;
		starItem.height = 60;
		starItem.visible = false;
	};

	// 最初に1回初期化を実行しておく
	InitGame();

	int imgPlayer = LoadGraph("image/Charactor.png");
	char KeyBuf[256];

	//起動直後にタイトルBGMをループ再生する
	PlaySoundMem(bgmTitle, DX_PLAYTYPE_LOOP);

	while (1)
	{
		ClearDrawScreen();
		GetHitKeyStateAll(KeyBuf);

		switch (Scene)
		{
		case SCENE_TITLE: //タイトル画面

				// 描画：背景だけ動かさずに表示
				DrawGraph(bgx, 0, imgBG, false);
				DrawGraph(bgx + WIDTH, 0, imgBG, false);

				// 描画：タイトル文字と案内
				DrawString(WIDTH / 2 - 120, HEIGHT / 2 - 100, "RUN GAME !!", GetColor(255, 255, 0));
				DrawString(WIDTH / 2 - 140, HEIGHT / 2 - 80, "初見殺しあり！！", GetColor(255, 0, 0));
				DrawFormatString(WIDTH / 2 - 120, HEIGHT / 2 - 30, GetColor(0, 255, 255), "HI-SCORE: %d", highScore);
				DrawString(WIDTH / 2 - 195, HEIGHT / 2 + 60, "Enterキーを押してゲームスタート", GetColor(255, 255, 255));

				//Enterキー（RETURN）が押されたらゲーム本編へ
				if (KeyBuf[KEY_INPUT_RETURN] == 1)
				{
					InitGame();     // ゲームの状態をリセット
					score = 0;

					// タイトルBGMを止めて、ゲームBGMをループ再生する
					StopSoundMem(bgmTitle);
					PlaySoundMem(bgmGame, DX_PLAYTYPE_LOOP);

					Scene = SCENE_GAME;  // シーンをゲーム本編に変える
				}
				break;

		case SCENE_GAME: // ゲーム本編

				score++;

				if (KeyBuf[KEY_INPUT_SPACE] == 1 && itemStock > 0 && invincibleTimer == 0)
				{
					itemStock--;
					invincibleTimer = INVINCIBLE_DURATION;
					PlaySoundMem(seHit, DX_PLAYTYPE_BACK);
				}

				if (invincibleTimer > 0)
				{
					invincibleTimer--;
				}

				if (score < 4000) { isReversed = false; }
				else { isReversed = (((score - 4000) / 1500) % 2 == 1); }

				// 背景のスクロール処理
				bgx = bgx - 5;
				if (bgx <= -WIDTH) bgx = 0;
				DrawGraph(bgx, 0, imgBG, false);
				DrawGraph(bgx + WIDTH, 0, imgBG, false);

				// プレイヤーの移動
				if (!isReversed)
				{
					if (KeyBuf[KEY_INPUT_D] == 1)
					{
						playerX += MOVE_SPEED;
						isRight = true;
					}
					if (KeyBuf[KEY_INPUT_A] == 1)
					{
						playerX -= MOVE_SPEED;
						isRight = false;
					}
				}
				else
				{
					if (KeyBuf[KEY_INPUT_D] == 1)
					{
						playerX -= MOVE_SPEED;
						isRight = false;
					}
					if (KeyBuf[KEY_INPUT_A] == 1)
					{
						playerX += MOVE_SPEED;
						isRight = true;
					}
				}
				if (playerX < 0) playerX = 0;
				if (playerX > WIDTH - playerWidth) playerX = WIDTH - playerWidth;

				// ジャンプ処理
				if (KeyBuf[KEY_INPUT_W] == 1 && !isJumping)
				{
					PlayerVY = JUMP_POWER;
					isJumping = true;
				}

				// 物理演算と着地
				PlayerVY += GRAVITY;
				playerY += PlayerVY;
				if (playerY + playerHeight >= GROUND_Y)
				{
					playerY = GROUND_Y - playerHeight;
					PlayerVY = 0;
					isJumping = false;
				}

				// アイテムの生成
				if (!starItem.visible && score % 500 == 0)
				{
					if (GetRand(9) < 2)
					{
						starItem.visible = true;
						starItem.x = WIDTH;
						starItem.y = GROUND_Y - starItem.height;
					}
				}

				if (starItem.visible)
				{
					starItem.x -= 5;

					if (CheckCollision(playerX, playerY, playerWidth, playerHeight, starItem.x, starItem.y, starItem.width, starItem.height))
					{
						itemStock++;
						starItem.visible = false;
					}

					if (starItem.x + starItem.width < 0)
					{
						starItem.visible = false;
					}

					DrawExtendGraph(starItem.x, starItem.y, starItem.x + starItem.width, starItem.y + starItem.height, imgItem, true);
				}

				// とげの生成
				togeTimer++;
				if (togeTimer >= TOGE_APPEAR_INTERVAL)
				{
					togeTimer = 0;
					for (int i = 0; i < MAX_TOGE; ++i)
					{
						if (!toges[i].visible)
						{
							toges[i].visible = true;
							toges[i].x = WIDTH;
							toges[i].type = (TogeType)GetRand(TOGE_MAX - 1);
							toges[i].y = GROUND_Y - togeHeights[toges[i].type];
							toges[i].vy = 0;
							toges[i].isLaunched = false;
							toges[i].isChecked = false;
							break;
						}
					}
				}

				// とげの移動・描画・当たり判定
				for (int i = 0; i < MAX_TOGE; ++i)
				{
					if (toges[i].visible)
					{
						if (!isReversed)
						{
							toges[i].x -= 5; // 左へ移動
						}
						else
						{
							toges[i].x -= 5;
						}

						int type = toges[i].type;

						if (type == TOGE_4 && !toges[i].isChecked)
						{
							if (toges[i].x > playerX && (toges[i].x - playerX) < 300)
							{
								int tmpRand = GetRand(9);
								if (tmpRand < 3)
								{
									toges[i].vy = -25;
									toges[i].isLaunched = true;
								}
								toges[i].isChecked = true;
							}
						}

						if (toges[i].isLaunched)
						{
							toges[i].y += toges[i].vy;
							toges[i].vy += 1;
						}

						if (toges[i].x + togeWidths[type] < 0)
						{
							toges[i].visible = false;
						}

						// 当たり判定：当たったらゲームオーバーシーンへ
						if (invincibleTimer == 0 && CheckCollision(playerX, playerY, playerWidth, playerHeight,
							toges[i].x, toges[i].y, togeWidths[type], togeHeights[type]))
						{
							if (score > highScore)
							{
								highScore = score;
							}

						// ゲームBGMを止め、とげヒットSEを1回再生し、ゲームオーバーBGMをループ再生する
						StopSoundMem(bgmGame);
						PlaySoundMem(seHit, DX_PLAYTYPE_BACK); // BACKは裏で1回だけ流す設定
						PlaySoundMem(bgmGameOver, DX_PLAYTYPE_LOOP);

						Scene = SCENE_GAMEOVER;
						}

						// とげの描画
						DrawExtendGraph(toges[i].x, toges[i].y,
							toges[i].x + togeWidths[type],
							toges[i].y + togeHeights[type],
							imgToge[type], TRUE);
					}
				}

				// プレイヤーの描画
				if (invincibleTimer > 0 && (invincibleTimer / 4) % 2 == 0){}
				else
				{
					if (isRight)
					{
						DrawExtendGraph(playerX + playerWidth, playerY, playerX, playerY + playerHeight, imgPlayer, true);
					}
					else
					{
						DrawExtendGraph(playerX, playerY, playerX + playerWidth, playerY + playerHeight, imgPlayer, true);
					}
				}

				DrawFormatString(50, 50, GetColor(255, 255, 255), "SCORE: %d", score);
				DrawFormatString(50, 90, GetColor(0, 255, 255), "HI-SCORE: %d", highScore);
				DrawFormatString(50, 130, GetColor(255, 255, 0), "ITEM STOCK: %d [Press Space to USE]", itemStock);
				if (invincibleTimer > 0)
				{
					DrawFormatString(50, 170, GetColor(255, 64, 64), "INVINCIBLE: %d", invincibleTimer);
				}

				if (isReversed)
				{
					SetFontSize(48);
					DrawString(WIDTH / 2 - 350, 100, "WARNING: REVERSE CONTROL!!", GetColor(255, 0, 0));
					SetFontSize(16);
				}
				break;

		case SCENE_GAMEOVER: // ゲームオーバー

				// 背景ととげは「その場に止まった状態」で描画
				DrawGraph(bgx, 0, imgBG, false);
				DrawGraph(bgx + WIDTH, 0, imgBG, false);
				for (int i = 0; i < MAX_TOGE; ++i)
				{
					if (toges[i].visible)
					{
						int type = toges[i].type;
						DrawExtendGraph(toges[i].x, toges[i].y, toges[i].x + togeWidths[type], toges[i].y + togeHeights[type], imgToge[type], TRUE);
					}
				}

				// ゲームオーバー画面用の文字表示
				DrawString(WIDTH / 2 - 120, HEIGHT / 2 - 100, "GAME OVER", GetColor(255, 0, 0));
				DrawFormatString(WIDTH / 2 - 140, HEIGHT / 2 - 60, GetColor(255, 255, 255), "YOUR SCORE: %d", score);
				DrawFormatString(WIDTH / 2 - 130, HEIGHT / 2 - 10, GetColor(0, 255, 255), "HI-SCORE: %d", highScore);
				DrawString(WIDTH / 2 - 200, HEIGHT / 2 + 40, "Spaceキーを押してタイトルへ戻る", GetColor(255, 255, 255));

				// Enterキーが押されたらタイトル画面に戻す
				if (KeyBuf[KEY_INPUT_SPACE] == 1)
				{
					// ゲームオーバーBGMを止め、タイトルBGMをループ再生する
					StopSoundMem(bgmGameOver);
					PlaySoundMem(bgmTitle, DX_PLAYTYPE_LOOP);

					Scene = SCENE_TITLE; // タイトルシーンに切り替え
				}
				break;
		}
			ScreenFlip();
			WaitTimer(16);
			if (ProcessMessage() == -1) break;
			if (KeyBuf[KEY_INPUT_ESCAPE] == 1) break;
	}
	DxLib_End();
	return 0;
}