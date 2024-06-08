# AviUtl 拡張編集フィルタプラグイン - パン

[AviUtl](http://spring-fragrance.mints.ne.jp/aviutl/)
で音声のパンニングを行うための拡張編集フィルタです。

## 導入方法

0. patch.aul (謎さうなフォーク版) r43_ss_58 以降を導入してください。
   - [patch.aul - nazosauna AviUtl](https://scrapbox.io/nazosauna/patch.aul)
1. [Releases](https://github.com/karoterra/aviutl_Panner/releases/)
   から最新版の ZIP ファイルをダウンロードしてください。
2. ZIP ファイルを展開し、以下のファイルを AviUtl の `plugins` フォルダに配置してください。
   - `PannerConfig.auf`
   - `PannerMain.eef`

## プロジェクトの設定

AviUtl の「表示」→「パンの表示」からウィンドウを表示してください。
このウィンドウで設定したPan Lawがプロジェクト内でのデフオルト値になります。

## パラメータ

- パン：パンニングの位置を設定します。-100 で左、0 で中央、100 で右です。
- Pan Law：パンニング法則を設定します。
  - プロジェクトの設定：プロジェクトの設定を使用します。
  - 0 dB：中央では 0 dB (原音と同じ) の振幅になります。左右どちらかいっぱいまで振り切ると、一方のチャネルは +3 dB、もう一方は無音になります。パンの全域にわたって両チャネルのパワーの和は一定に保たれます。
  - -3 dB：中央では -3 dB の振幅になります。左右どちらかいっぱいまで振り切ると、一方のチャネルは 0 dB、もう一方は無音になります。パンの全域にわたって両チャネルのパワーの和は一定に保たれます。
  - -4.5 dB：中央では -4.5 dB の振幅になります。左右どちらかいっぱいまで振り切ると、一方のチャネルは 0 dB、もう一方は無音になります。
  - -6 dB：中央では -6 dB の振幅になります。左右どちらかいっぱいまで振り切ると、一方のチャネルは 0 dB、もう一方は無音になります。

## ライセンス

[MIT License](LICENSE) に基づきます。

また、以下のライブラリを使用しています。

### aviutl_exedit_sdk v1.2 (謎さうなフォーク版)

https://github.com/nazonoSAUNA/aviutl_exedit_sdk/tree/command

```
Copyright (c) 2022
ePi All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
THIS SOFTWARE IS PROVIDED BY ePi “AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ePi BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

## 更新履歴
