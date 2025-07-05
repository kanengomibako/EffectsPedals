#include "main.h"
#include "user_main.h"
#include "fonts.h"
#include "fx.hpp"
#include "lcd.hpp"
#include "calc.hpp"

extern SAI_HandleTypeDef hsai_BlockA1;
extern SAI_HandleTypeDef hsai_BlockB1;

volatile int32_t RX_BUFFER[2][BLOCK_SIZE*2] = {}; // 音声信号受信バッファ
volatile int32_t TX_BUFFER[2][BLOCK_SIZE*2] = {}; // 音声信号送信バッファ
uint8_t BUFnum = 0; // バッファを0と1の2つ準備

uint8_t footSW = {}; // フットスイッチオン・オフ状態
uint8_t touchFlag = 0;  // タッチパネルフラグ 1: 短押し 2: 長押し

uint16_t FXparam[20] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}; // 現在のエフェクトパラメータ
uint16_t FXparamMax[20] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}; // エフェクトパラメータ最大値
uint16_t FXparamMin[20] = {}; // エフェクトパラメータ最小値
std::string FXparamName[20] = {}; // エフェクトパラメータ名 LEVEL, GAIN等
std::string FXparamStr[20] = {};  // エフェクトパラメータ数値 文字列
uint8_t FXpage = 0; // エフェクトパラメータ 現在のページ
uint8_t FXpageMax = 0; // エフェクトパラメータ ページ数最大値

uint8_t FXnum = 0; // 現在のエフェクト番号
int8_t FXchangeFlag = 0; // エフェクト種類変更フラグ 次エフェクトへ: 1 前エフェクトへ: -1

uint16_t allData[MAX_FX_NUM][20] = {}; // エフェクトデータ配列

const uint32_t flash_addr = 0x081E0000; // データ保存先（バンク2 セクタ7）開始アドレス

const uint8_t FXnameX = 3; // エフェクト名表示位置
const uint8_t FXnameY = 2;
const uint8_t FXparamStrX[6]   = {  0, 81,162,  0, 81,162}; // エフェクトパラメータ表示位置
const uint16_t FXparamStrY[6]  = {137,137,137,257,257,257};
const uint8_t circleX[6]       = { 38,119,200, 38,119,200}; // パラメータ 円の位置
const uint8_t circleY[6]       = {100,100,100,220,220,220};
const uint8_t FXparamNameX[6]  = {  0, 81,162,  0, 81,162}; // エフェクトパラメータ名表示位置
const uint8_t FXparamNameY[6]  = { 42, 42, 42,166,166,166};

float cycle = 10000000; // 時間計測用

void mainInit() // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<最初に1回のみ行う処理
{
  // 時間計測用設定
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  //DWT->CYCCNT = 0;
  //cycle += (float)DWT->CYCCNT / 480000.0f;

  // LCD初期化
  HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_SET); // ILI9341_Unselect
  HAL_GPIO_WritePin(ILI9341_TOUCH_CS_GPIO_Port, ILI9341_TOUCH_CS_Pin, GPIO_PIN_SET); // ILI9341_TouchUnselect
  ILI9341_Init();
  ILI9341_WriteString(0, 0, "STARTING...", Font_16x26, ILI9341_BLACK, ILI9341_WHITE);

  // 基板上のLED点灯（起動確認）
  HAL_GPIO_WritePin(BOARD_LED_GPIO_Port, BOARD_LED_Pin, GPIO_PIN_SET);

  // 保存済パラメータ読込
  loadData();

  // 初期描画
  ILI9341_FillScreen(ILI9341_BLACK);
  for (int i = 0; i < 6; i++) ILI9341_Circle(circleX[i], circleY[i], 35, ILI9341_WHITE);
  ILI9341_WriteString(58, 310, PEDAL_NAME, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);

  // 初回表示エフェクト初期化
  FXinit();

  // SAI 通信開始
  HAL_SAI_Receive_IT(&hsai_BlockA1, (uint8_t *)RX_BUFFER[BUFnum], BLOCK_SIZE*2);

  // オーディオコーデックオン
  HAL_GPIO_WritePin(CODEC_RST_GPIO_Port, CODEC_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(100);

  // SAIのエラー検出時、SAIとオーディオコーデックをリセット
  while(__HAL_SAI_GET_FLAG(&hsai_BlockA1, SAI_FLAG_AFSDET)
      || __HAL_SAI_GET_FLAG(&hsai_BlockA1, SAI_FLAG_LFSDET)
      || __HAL_SAI_GET_FLAG(&hsai_BlockB1, SAI_FLAG_AFSDET)
      || __HAL_SAI_GET_FLAG(&hsai_BlockB1, SAI_FLAG_LFSDET))
  {
    ILI9341_WriteString(0, 310, "ERROR!", Font_7x10, ILI9341_WHITE, ILI9341_RED);
    HAL_SAI_Abort(&hsai_BlockA1);
    HAL_SAI_Abort(&hsai_BlockB1);
    HAL_GPIO_WritePin(CODEC_RST_GPIO_Port, CODEC_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_SAI_Receive_IT(&hsai_BlockA1, (uint8_t *)RX_BUFFER[BUFnum], BLOCK_SIZE*2);
    HAL_GPIO_WritePin(CODEC_RST_GPIO_Port, CODEC_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
  }

}

void mainLoop() // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<メインループ
{
  static uint16_t last_FXparam[6] = {}; // 前回エフェクトパラメータ配列
  static std::string last_FXparamStr[6] = {}; // 前回エフェクトパラメータ文字列配列
  static std::string last_FXparamName[6] = {}; // 前回エフェクトパラメータ名称配列
  static uint8_t FXparamChangeFlag = 0; // エフェクトパラメータ数値表示変更フラグ

  static uint8_t last_footSW = 5;   // 前回スイッチオン・オフ状態
  static uint8_t last_FXnum = 99;   // 前回エフェクト番号
  static uint8_t last_FXpage = 99;  // 前回エフェクトパラメータページ
  static uint16_t x, y; // タッチ位置x, y

  const int8_t d7[8] = {0, 33, 27, 22, 16, 11, 5, 0}; // 位置調整値 中央揃え表示 7文字用
  const int8_t d9[10] = {0, 44, 38, 33, 27, 22, 16, 11, 5, 0}; // 位置調整値 中央揃え表示 9文字用
  const int8_t d9b[10] = {0, 64, 56, 48, 40, 32, 24, 16, 8, 0}; // 位置調整値 中央揃え表示 9文字 大フォント用
  uint8_t len = 0; // パラメータ名文字列長さ 0～7

  static float x0[6] = {}, y0[6] = {}, last_x0[6] = {}, last_y0[6] = {}; // 三角形用位置配列
  static float x1[6] = {}, y1[6] = {}, last_x1[6] = {}, last_y1[6] = {};
  static float x2[6] = {}, y2[6] = {}, last_x2[6] = {}, last_y2[6] = {};

  if (touchFlag == 1) // タッチ位置取得------------------------------
  {
    ILI9341_TouchGetXY(x, y);
    touchFlag = 0;
    //ILI9341_DrawPixel(x ,y , ILI9341_RED); // タッチ位置確認用
  }
  else if (touchFlag == 2) // タッチパネル短押し
  {
    touchProcess(x, y, 1);
    touchFlag = 0;
  }
  else if (touchFlag == 3) // タッチパネル長押し
  {
    touchProcess(x, y, 10);
    touchFlag = 0;
  }

  if (last_footSW != footSW) // エフェクトオン・オフ表示切替------------------------------
  {
    if (footSW != 0)
    {
      ILI9341_FillRectangle(187, 0, 42, 28, ILI9341_RED);
      ILI9341_WriteString(192, 2, "ON", Font_16x26, ILI9341_BLACK, ILI9341_RED);
    }
    else
    {
      ILI9341_FillRectangle(187, 0, 42, 28, ILI9341_COLOR565(96, 96, 96));
      ILI9341_WriteString(192, 6, "OFF", Font_11x18, ILI9341_BLACK, ILI9341_COLOR565(96, 96, 96));
    }
    last_footSW = footSW;
  }

  for (int i = 0; i < 6; i++) // エフェクトパラメータ数値表示変更------------------------------
  {
    if (last_FXparam[i] != FXparam[i+6*FXpage] || FXparamChangeFlag)
    {
      FXsetParamStr(i+6*FXpage); // パラメータを文字列に変換

      if (last_FXparamStr[i] != "") // 前回表示を消す
      {
        len = (uint8_t)last_FXparamStr[i].length(); // パラメータ数値文字列長さ 1～7
        if (len > 7) len = 7;
        ILI9341_FillRectangle(FXparamStrX[i] + d7[len], FXparamStrY[i], len * 11, 18, ILI9341_BLACK);
      }

      // 前回三角形表示を消す
      ILI9341_FillTriangle(last_x0[i], last_y0[i], last_x1[i], last_y1[i], last_x2[i], last_y2[i], ILI9341_BLACK);

      if (FXparamStr[i+6*FXpage] != "") // 今回表示
      {
        len = (uint8_t)FXparamStr[i+6*FXpage].length();
        if (len > 7) len = 7;
        ILI9341_WriteString(FXparamStrX[i] + d7[len], FXparamStrY[i], FXparamStr[i+6*FXpage].c_str(), Font_11x18, ILI9341_WHITE, ILI9341_BLACK);

        float paramPercent = 100.0f * (float)(FXparam[i+6*FXpage]  - FXparamMin[i+6*FXpage]) / (float)(FXparamMax[i+6*FXpage] - FXparamMin[i+6*FXpage]);
        float theta = 2.0f * PI * (120.0f / 360.0f + 3.0f * paramPercent / 360.0f);
        x0[i] = (float)circleX[i] + 33.0f * cosf(theta) + 0.5f;
        y0[i] = (float)circleY[i] + 33.0f * sinf(theta) + 0.5f;
        x1[i] = x0[i] + 20.0f * cosf(2.0f*PI*(180.0f-20.0f)/360.0f - theta) + 0.5f;
        y1[i] = y0[i] - 20.0f * sinf(2.0f*PI*(180.0f-20.0f)/360.0f - theta) + 0.5f;
        x2[i] = x0[i] + 20.0f * cosf(2.0f*PI*(180.0f+20.0f)/360.0f - theta) + 0.5f;
        y2[i] = y0[i] - 20.0f * sinf(2.0f*PI*(180.0f+20.0f)/360.0f - theta) + 0.5f;
        ILI9341_FillTriangle(x0[i], y0[i], x1[i], y1[i], x2[i], y2[i], FXcolorList[FXnum]);
      }
      last_x0[i] = x0[i];
      last_y0[i] = y0[i];
      last_x1[i] = x1[i];
      last_y1[i] = y1[i];
      last_x2[i] = x2[i];
      last_y2[i] = y2[i];
      last_FXparam[i] = FXparam[i+6*FXpage];
      last_FXparamStr[i] = FXparamStr[i+6*FXpage];
      if (i == 5) FXparamChangeFlag = 0;
    }
  }

  if (last_FXnum != FXnum) // エフェクト表示変更------------------------------
  {
    ILI9341_FillRectangle(0, 0, 151, 28, FXcolorList[FXnum]);
    uint8_t nextFXnum = clipiCirc(FXnum + 1 , 0, MAX_FX_NUM - 1);
    uint8_t prevFXnum = clipiCirc(FXnum - 1 , 0, MAX_FX_NUM - 1);
    ILI9341_FillRectangle(11, 286, 100, 21, FXcolorList[prevFXnum]);
    ILI9341_FillTriangle(0, 296, 10, 286, 10, 306, FXcolorList[prevFXnum]);
    ILI9341_FillRectangle(129, 286, 100, 21, FXcolorList[nextFXnum]);
    ILI9341_FillTriangle(229, 286, 229, 306, 239, 296, FXcolorList[nextFXnum]);
    len = (uint8_t)FXnameList[prevFXnum].length();
    if (len > 9) len = 9;
    ILI9341_WriteString(11 + d9[len], 289, FXnameList[prevFXnum].c_str(), Font_11x18, ILI9341_BLACK, FXcolorList[prevFXnum]);
    len = (uint8_t)FXnameList[nextFXnum].length();
    if (len > 9) len = 9;
    ILI9341_WriteString(131 + d9[len], 289, FXnameList[nextFXnum].c_str(), Font_11x18, ILI9341_BLACK, FXcolorList[nextFXnum]);
    len = (uint8_t)FXnameList[FXnum].length();
    if (len > 9) len = 9;
    ILI9341_WriteString(FXnameX + d9b[len], FXnameY, FXnameList[FXnum].c_str(), Font_16x26, ILI9341_BLACK, FXcolorList[FXnum]);
    last_FXnum = FXnum;
    FXparamChangeFlag = 1;
  }

  for (int i = 0; i < 6; i++) // エフェクトパラメータ名称表示変更------------------------------
  {
    if (last_FXparamName[i] != FXparamName[i+6*FXpage])
    {
      if (last_FXparamName[i] != "") // 黒い四角で前回表示を消す
      {
        len = (uint8_t)last_FXparamName[i].length(); // パラメータ名文字列長さ 1～7
        if (len > 7) len = 7;
        ILI9341_FillRectangle(FXparamNameX[i] + d7[len], FXparamNameY[i], len * 11, 18, ILI9341_BLACK);
      }

      if (FXparamName[i+6*FXpage] == "") // 使わないパラメータの場合、パラメータ数値表示を消す
      {
        FXparamStr[i+6*FXpage] = "";
      }
      else // 今回表示
      {
        len = (uint8_t)FXparamName[i+6*FXpage].length();
        if (len > 7) len = 7;
        ILI9341_WriteString(FXparamNameX[i] + d7[len], FXparamNameY[i], FXparamName[i+6*FXpage].c_str(), Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
      }
      last_FXparamName[i] = FXparamName[i+6*FXpage];
    }
  }

  if (last_FXpage != FXpage) // エフェクトパラメータページ番号表示変更------------------------------
  {
    std::string FXpageStr = "P" + std::to_string(FXpage + 1);
    ILI9341_WriteString(159, 6, FXpageStr.c_str(), Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
    last_FXpage = FXpage;
    FXparamChangeFlag = 1;
  }

}

void mute() // エフェクト切替時、データ保存時のミュート
{
  for (int i = 0; i < BLOCK_SIZE*2; i++)
  {
    TX_BUFFER[0][i] = 0;
    TX_BUFFER[1][i] = 0;
  }
}

inline void footsw() // フットスイッチ
{
  static uint32_t fscount = 0;
  if (!HAL_GPIO_ReadPin(F_SW_GPIO_Port, F_SW_Pin))
  {
    fscount++;
    if (fscount == 20) footSW = !footSW;
  }
  else fscount = 0;
}

void touchProcess(uint16_t x, uint16_t y, uint8_t a) // タッチ位置による動作分岐
{
  uint8_t num = 0; // ポット番号
  if (y < 25) // LCD最上段
  {
    if (a == 1) FXpage = clipiCirc(FXpage + 1, 0, FXpageMax); // 短押しでページ切替
    else if (a > 1 && y > 0) saveData(); // 長押しでデータ保存
  }
  else if (y < 100) // LCD上段パラメータ増加
  {
    if      (x <  80) num = 0 + 6 * FXpage;
    else if (x < 160) num = 1 + 6 * FXpage;
    else              num = 2 + 6 * FXpage;
    FXparam[num] = clipi(FXparam[num] + a, FXparamMin[num], FXparamMax[num]);
  }
  else if (y < 160) // LCD上段パラメータ減少
  {
    if      (x <  80) num = 0 + 6 * FXpage;
    else if (x < 160) num = 1 + 6 * FXpage;
    else              num = 2 + 6 * FXpage;
    FXparam[num] = clipi(FXparam[num] - a, FXparamMin[num], FXparamMax[num]);
  }
  else if (y < 220) // LCD下段パラメータ増加
  {
    if      (x <  80) num = 3 + 6 * FXpage;
    else if (x < 160) num = 4 + 6 * FXpage;
    else              num = 5 + 6 * FXpage;
    FXparam[num] = clipi(FXparam[num] + a, FXparamMin[num], FXparamMax[num]);
  }
  else if (y < 280) // LCD下段パラメータ減少
  {
    if      (x <  80) num = 3 + 6 * FXpage;
    else if (x < 160) num = 4 + 6 * FXpage;
    else              num = 5 + 6 * FXpage;
    FXparam[num] = clipi(FXparam[num] - a, FXparamMin[num], FXparamMax[num]);
  }
  else if (a > 1) // LCD最下段 長押しでエフェクト切替
  {
    if (x > 120) FXchangeFlag = 1;
    else FXchangeFlag = -1;
  }
}

void FXchange() // エフェクト変更
{ // ※ディレイメモリ確保前に信号処理に進まないよう割り込み内で行う
  mute();
  FXdeinit();
  FXnum = clipiCirc(FXnum + FXchangeFlag, 0 ,MAX_FX_NUM - 1);
  FXpage = 0;
  FXinit();
  FXchangeFlag = 0;
}

inline void touchsw()  // タッチパネル
{
  static uint32_t touchCount = 0;
  if (!HAL_GPIO_ReadPin(T_SW_GPIO_Port, T_SW_Pin))
  {
    touchCount++;
    if (touchCount == 10) touchFlag = 1; // タッチ位置を取得
    else if (touchCount == 700) touchFlag = 3; // 長押し
    else if (touchCount > 900 && (touchCount % 300) == 299) touchFlag = 3; // 押し続けたとき
  }
  else
  {
    if (touchCount >= 10 && touchCount < 700) touchFlag = 2; // 短押し
    touchCount = 0;
  }
}

inline void mainProcess()
{
  static float xL[BLOCK_SIZE] = {}; // Lch float計算用データ
  static float xR[BLOCK_SIZE] = {}; // Rch float計算用データ

  // 受信データを計算用データ配列へ 値を-1～+1(float)へ変更
  for (int i = 0; i < BLOCK_SIZE; i++)
  {
    xL[i] = (float)RX_BUFFER[BUFnum][i*2] / 2147483648.0f;
    xR[i] = (float)RX_BUFFER[BUFnum][i*2+1] / 2147483648.0f;
  }

  // エフェクト処理 計算用配列を渡す
  FXprocess(xL, xR);

  for (int i = 0; i < BLOCK_SIZE; i++)
  {
    // オーバーフロー防止 実測0.998まで
    xL[i] = clipf(xL[i], -1.0f, 0.998f);
    xR[i] = clipf(xR[i], -1.0f, 0.998f);

    // 計算済データを送信バッファへ 値を32ビット整数へ戻す
    TX_BUFFER[BUFnum][i*2] = (int32_t)(2147483648.0f * xL[i]);
    TX_BUFFER[BUFnum][i*2+1] = (int32_t)(2147483648.0f * xR[i]);
  }
}

// SAI受信完了割込 割込間のサイクル数348288 = 0.7256ms実測済
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
  HAL_SAI_Receive_IT(&hsai_BlockA1, (uint8_t *)RX_BUFFER[invert(BUFnum)], BLOCK_SIZE*2);
  HAL_SAI_Transmit_IT(&hsai_BlockB1, (uint8_t *)TX_BUFFER[invert(BUFnum)], BLOCK_SIZE*2);

  if (FXchangeFlag) FXchange(); // エフェクト切替
  mainProcess();
  BUFnum = invert(BUFnum);
  footsw();
  touchsw();
}

void loadData()
{
  uint32_t addr = flash_addr;
  for (int i = 0; i < MAX_FX_NUM; i++)
  {
    for (int j = 0; j < 20; j++)
    {
      allData[i][j] = *((uint16_t*)addr);
      if (allData[i][j] > 10000) allData[i][j] = 50;
      addr += 2;
    }
  }
}

void saveData()
{
  mute();
  ILI9341_WriteString(58, 310, "  WRITING DATA...  ", Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  HAL_FLASH_Unlock(); // フラッシュ ロック解除

  /* フラッシュ 消去 */
  FLASH_EraseInitTypeDef erase;               // 消去に関する構造体を定義
  uint32_t error = 0;                         // エラーコードを格納する変数
  erase.TypeErase = FLASH_TYPEERASE_SECTORS;  // 消去方法: セクタ消去
  erase.Banks = FLASH_BANK_2;                 // 消去するバンク: バンク2
  erase.Sector = FLASH_SECTOR_7;              // 消去するセクタ: セクタ7
  erase.NbSectors = 1;                        // 消去するセクタの数: 1
  erase.VoltageRange = FLASH_VOLTAGE_RANGE_3; // 電圧設定
  HAL_FLASHEx_Erase(&erase, &error);          // 消去

  uint32_t addr = flash_addr;
  uint16_t tmpArray[16] = {}; // フラッシュ書き込み用256ビット配列
  uint8_t dataCount = 0;

  for (int i = 0; i < 20; i++) // 現在のパラメータを保存用配列へ移す
  {
    allData[FXnum][i] = FXparam[i];
  }

  for (int i = 0; i < MAX_FX_NUM; i++) // 全データ フラッシュ書込
  {
    for (int j = 0; j < 20; j++)
    {
      tmpArray[dataCount] = allData[i][j];
      dataCount++;
      if (dataCount == 16 || (i == MAX_FX_NUM - 1 && j == 5)) // データが16個たまったときまたは最後のデータでフラッシュ書込
      {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, addr, (uint32_t)tmpArray);
        addr += 32;
        dataCount = 0;
      }
    }
  }

  HAL_FLASH_Lock(); // フラッシュ ロック
  ILI9341_WriteString(58, 310, "    DATA SAVED!     ", Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
  for (volatile uint32_t i = 0; i < 20000000; i++){}; // 割り込み内ウェイト
  ILI9341_WriteString(58, 310, PEDAL_NAME, Font_7x10, ILI9341_WHITE, ILI9341_BLACK);
}
