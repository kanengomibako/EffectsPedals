Owm Board

Owm Pedalに用いているDSPボードです。
製作記事→https://kanengomibako.github.io/pages/00237.html

＜更新履歴＞
Rev.C 2019年6月
・入力に抵抗を追加
・V4220Mをリセットできるようマイコンと接続
・U7(7805)、U9(7833)の放熱部をスルーホールへ変更
・U5をSTM32F722とSTM32F730に対応可能なよう周辺回路を変更
※STM32F7での追加結線等は最下部参照

Rev.B 2019年2月
・BIASへの接続ミスを修正

Rev.A 2018年12月

＜パーツリスト（Rev.C）＞
20p*	4	[1608] C15, C16, C17, C18 *水晶振動子により容量が変わる
220p	4	[1608] C7, C8, C12, C13
1n	4	[1608] C5, C6, C10, C11
10n	2	[1608] C2, C4
100n	16	[1608] C22, C23, C24, C25, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C42
100n*	2	[3225] C1,C3 *PMLCAP
2u2	2	[2012] C19, C20
10u	5	[3216] C26, C27, C28, C29, C43
10u*	2	C9, C14 *SMD電解コンデンサ（直径5mm）
100u*	2	C21, C41 *DIP電解コンデンサ（直径6.3mm）

100R	6	[2012] R5, R6, R11, R12, R21, R31
1k	3	[2012] R1, R7, R38
2.2k	4	[1608] R19, R20, R29, R30
10k	21	[1608] R3, R4, R9, R10, R13, R14, R15, R16, R17, R18, R23, R24, R25, R26, R27, R28, R34, R36, R39, R41, R42 
47k	1	[1608] R33
100k	4	[1608] R22, R32, R43, R44
1M	2	[1608] R2, R8
2.2M	2	[1608] R35, R45

330R*	1	[2012] FB1 *SMDフェライトビーズ BLM21PG331SN
		
8MHz		1	Y2
12.288MHz	1	Y1
		
LED	1	D1
1N4007	1	D2
		
OPA1678IDR	3	U1, U2, U3
V4220M		1	U4
STM32F405RGT6	1	U5
NJM78L33SU3	1	U10
NJM2845DL1-33	1	U9
NJM78L05SU3	2	U6, U8
NJM7805SDL1	1	U7

[ F405 -> F722 or F730 ]
C19: 2.2uF -> Jumper
C20: 2.2uF -> Jumper
C44: Nothing -> 4.7uF
Not compatible pins:
PC5, PB0, PB1, PB2, PB10, PB11
※STM32F7では以下のようにテストポイントとピンの結線が必要
LRCK→PA15
BCK→PC10
ADC→PC12
