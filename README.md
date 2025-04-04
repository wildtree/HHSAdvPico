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

## 予定
最初はおそらく Pico2/2W向けのものが出てくると思います。
そのあと、Pico用のビルド情報が追加されて、気分が乗っていれば Pico W/2Wに液晶モジュールをつないで、BLEキーボードで遊べる奴が出てくるかもしれません。

## 制限
結局のところ、現状では Raspberry Pi Pico 2/2Wでないと動きません。
264KBのメモリーではどうも足りないようです。
プロジェクトのターゲットにはいろいろありますが、動作確認が取れているのは `env:pico2` (PicoCalc版)のみです。

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
わたしの小さなデモプログラムは何度でも動いたので、てっきりわたしのプログラムが何か想定外のことをしてるんだろうと思い、三日ほど戦い続けることとなりましｔが、デバッガー、偉大です。

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
