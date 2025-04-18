# HHSAdvPico
High High School Adventure for PicoCalc

## 概要
いつもの、ハイハイスクールアドベンチャーの移植版です。
[PicoCalc](https://www.clockworkpi.com/picocalc) 向けのものです。

## 遊び方
ターゲットpico2 を選び、Raspberry Pi Pico2/2Wを入れたPicoCalcにビルドした firmware.uf2を書きこんでください。
data フォルダの中身を、SDカードのルートフォルダにコピーしてください。
ルートフォルダに `HHSAdv` というフォルダーが中身ごとコピーされるようにしてください。

あとは、いつものように、さまよってください。

## ビルドターゲットと対応環境

|ターゲット|対応する環境|ボード|
|:--------|:----------|:-----|
|env:pico1|PicoCalc|Pico/Pico W|
|env:pico2|PicoCalc|Pico2/Pico2w|
|env:pico1_lcd28|Pico Res-Touch LCD 2.8 + USBキーボード|Pico/Pico W|
|env:pico2_lcd28|Pico Res-Touch LCD 2.8 + USBキーボード|Pico2/Pico2W|
|env:pico1_lcd28_ble|Pico Res-Touch LCD 2.8 + BLEキーボード|Pico W|
|env:pico2_lcd28_ble|Pico Res-Touch LCD 2.8 + BLEキーボード|Pico2W|
|env:pico1_lcd35|Pico Res-Touch LCD 3.5 + USBキーボード|Pico/Pico W|
|env:pico2_lcd35|Pico Res-Touch LCD 3.5 + USBキーボード|Pico2/Pico2W|
|env:pico1_lcd35_ble|Pico Res-Touch LCD 3.5 + BLEキーボード|Pico W|
|env:pico2_lcd35_ble|Pico Res-Touch LCD 3.5 + BLEキーボード|Pico2W|

Pico w + BLEキーボードの組み合わせはちょっと動きが怪しいので推奨しません。

## 予定
~~Pico Res-Touch LCD 3.5も発注したので、到着したらそれ用のコードも追加する予定です。~~

## 制限

プロジェクトのターゲットにはいろいろありますが、`env:pico1_lcd28_ble` とデバッグ用のターゲット以外ではクリアまで動作確認できています。

## LovyanGFX 1.2.0について
:::note info
LovyanGFXの問題については、[らびやんさん](https://x.com/lovyan03)が速攻で修正してくださいました。
developブランチが修正されているので、現在、developブランチから LovyanGFXを取得するようにしています。
将来的に正式版が修正されたら、そちらを参照するように再度修正します。
:::

PicoCalcの電源を切って入れなおすと動かない問題があり、なかなか公開に至らなかったのですが、デバッグプローブを買って、デバッガーつないだらあっさりと原因は判明しました。
つまるところ、LovyanGFXのバグでした。

Bus_SPI::init() の中で、_spi_regs というポインタにアクセスするのですが、これが未定義で落ちます。
フラッシュに書きこんだ直後だけおそらくいい感じに例外をはかないところをポイントしてて、以後は運しだいだったのでしょう。
わたしの小さなデモプログラムは何度でも動いたので、てっきりわたしのプログラムが何か想定外のことをしてるんだろうと思い、三日ほど戦い続けることとなりましたが、デバッガー、偉大です。

なので、LovyanGFX 1.2.0に対して、以下の修正を入れてから、ビルドしてください。

.pio/pico2/LovyanGFX/src/lgfx/v1/platforms/rp2040/Bus_SPI.cpp
```diff_cpp
--- Bus_SPI.cpp.orig    2024-11-22 16:12:20.561052300 +0900
+++ Bus_SPI.cpp 2025-04-04 16:13:20.533910740 +0900
@@ -60,14 +60,14 @@
       return false;
     }

-    uint32_t temp = _spi_regs->cr0 & ~(SPI_SSPCR0_SCR_BITS | SPI_SSPCR0_DSS_BITS);
-    _clkdiv_write |= temp;
-    _clkdiv_read  |= temp;
-
     // DCピンを出力に設定
     lgfxPinMode(_cfg.pin_dc, pin_mode_t::output);
     _spi_regs = reinterpret_cast<spi_hw_t *>(_spi_dev[_cfg.spi_host]);

+    uint32_t temp = _spi_regs->cr0 & ~(SPI_SSPCR0_SCR_BITS | SPI_SSPCR0_DSS_BITS);
+    _clkdiv_write |= temp;
+    _clkdiv_read  |= temp;
+
     int dma_ch = dma_claim_unused_channel(true);
     _dma_ch = dma_ch;
     if (dma_ch >= 0)
```

それはさておき、迅速に開発ができたのは間違いなくLovyanGFXのおかげです。らびやんさんに感謝！

## TinyUSBについて

TinyUSBは大変便利なライブラリですが、ちょっと困ったバグがあります。コールバックをWEAK属性でライブラリ内に用意しておき、ユーザが定義した関数で上書きできるようにデザインしたつもりらしいですが、困ったことに、実装(*.c)のみならず、宣言(*.h)にもWEAKをつけちゃっているので、ユーザが定義したコールバックも全部WEAK属性にされてしまいます。

なので、ユーザ定義のコールバックとライブラリのデフォルトのコールバックのどちらが採用されるのかは運次第((リンクの順序次第))になってしまっています。

回避方法として、わたしは、TinyUSB関連のヘッダを読み込まないUSBKBD.cppというファイルを作って、そこでコールバックを非WEAKにして、そこから、キーボード処理のほうに定義した自分のコールバック本体を呼び出す形としました。
WEAK属性は便利ですが使い方を間違えるとみんなで不幸になるので、register callbackする方が今どきはわかりやすいのかなと思います。
C++に限れば、ラムダ式を使うという手もありますので、WEAKみたいなやり方はいまいちかなと思います。

## BLEキーボードについて

BLEキーボードは、M5ATOMで使ったので、簡単だろうと甘く見ていました。
簡単だったのはNimBLEがあったからです。
NimBLE Arduinoは残念ながら Raspberry Pi Picoをサポートしていません。

代わりに、btstack 1.6.2がSDKに入っています。
ただ、このライブラリはAPIむき出しなので、決して便利ではありません。
作法について学んで、順序良く処理を書いていかないと、GATTからキーボードのNoificationを受けることはできないのです。

そして、例によって、この手のデバイスを、BLEペリフェラルにする用例は豊富にあっても、BLEセントラルにする用例は少なく、手探りでの開発となりました。
デバッガが使えたのが本当に幸いで、デバッガがなければ動作させるには至らなかったと思います。
