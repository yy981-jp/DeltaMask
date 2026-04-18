# DeltaMask

**DeltaMask** は、PNG 画像に任意のバイナリデータを秘匿埋め込みするステガノグラフィツールです。  
元画像をキーとして使用し、ChaCha20 PRNG によるピクセルシャッフルで埋め込み位置を分散させることで、目視による検出を困難にします。

---

## 特徴

- **Delta 方式の埋め込み** — 各チャンネル値に `±1` または `±2` の微小な差分を加えてビットを表現
- **元画像がキー** — 元画像の SHA-256 ハッシュをシードとした ChaCha20 PRNG でピクセルアクセス順をシャッフル。元画像なしではデコード不可
- **2 つのビットモード** — Bit1（1ch あたり 1 bit）と Bit2（1ch あたり 2 bit）
- **整合性検証** — CRC32 によって復元データの破損を検出
- **拡張子の保持** — ヘッダにオリジナルの拡張子を記録し、復元時に自動で付与
- **依存ライブラリ最小** — libsodium + stb_image のみ

---

## アルゴリズム概要

```
[元画像] --SHA-256--> seed
seed --ChaCha20--> シャッフルされたピクセル列
埋め込みデータ --BitStream--> 各ピクセルの R/G/B チャンネルに delta 書き込み
[出力画像 .dm.png]
```

### チャンネル使用条件

極端な値（クリッピング）を避けるため、以下の範囲のチャンネルのみ使用します。

| モード | 使用可能範囲 |
|--------|-------------|
| Bit1   | `1 ≤ v ≤ 254` |
| Bit2   | `2 ≤ v ≤ 253` |

### ヘッダ構造（`#pragma pack(1)`）

| フィールド | サイズ | 内容 |
|-----------|--------|------|
| `magic`   | 4 B    | `0x5944544D` ("YDTM") |
| `version` | 1 B    | 現在 `2` |
| `flags`   | 1 B    | 予約済み |
| `bitSize` | 4 B    | 埋め込みデータのビット数 |
| `crc`     | 4 B    | CRC32 チェックサム |
| `ext`     | 8 B    | 元ファイルの拡張子（最大 7 文字） |

---

## ビルド

### 依存関係

- CMake ≥ 3.20
- C++23 対応コンパイラ（GCC 13 / Clang 17 以降推奨）
- [libsodium](https://libsodium.org/)
- [stb](https://github.com/nothings/stb)（`external/` に submodule として含む）
- [yy981/tools](https://github.com/yy981-jp/tools)（`external/` に submodule として含む）

### 手順

```bash
git clone --recursive https://github.com/yy981-jp/DeltaMask
cd DeltaMask
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

ビルド成果物は `build/DM`（Linux）または `build/DM.exe`（Windows）に生成されます。

---

## 使い方

```
DeltaMask v1.1 (c) 2026 yy981
  DM enc  <mode> <image.png> <data.dat>
  DM dec  <mode> <original.png> <encoded.png>
  DM info <mode> <image.png>
  DM info <mode> <original.png> <encoded.png>
         ^ -- bit mode {1, 2}
```

### エンコード（データを画像に埋め込む）

```bash
# Bit1 モードで example.dat を cover.png に埋め込む
DM enc 1 cover.png example.dat
# → cover.dm.png が生成される
```

### デコード（画像からデータを取り出す）

```bash
# 元画像と埋め込み済み画像を指定してデコード
DM dec 1 cover.png cover.dm.png
# → cover.dmo.dat が生成される
```

> **注意**: デコードには必ず**元画像**（エンコード時に使用したもの）が必要です。

### 容量確認

```bash
# 画像の埋め込み可能容量を確認
DM info 1 cover.png

# 埋め込み済み画像のヘッダ情報を表示
DM info 1 cover.png cover.dm.png
```

出力例：
```
Usable: 1.23 MiB
```

---

## ビットモードの比較

| モード | 1 チャンネルあたり | 画質への影響 | 容量 |
|--------|-------------------|-------------|------|
| Bit1   | 1 bit（±1 delta）| 最小        | 低   |
| Bit2   | 2 bit（±2 delta）| やや大きい  | 高   |

高解像度・高品質な写真には Bit1 を推奨します。

---

## ファイル構成

```
DeltaMask/
├── CMakeLists.txt
├── src/
│   ├── main.cpp          # エントリポイント・CLI
│   ├── core.h            # encode / decode / info 実装
│   ├── def.h             # 型定義・Header 構造体・Mode enum
│   ├── util.h            # Delta 演算・ビット変換ユーティリティ
│   ├── hashutil.h        # SHA-256 / CRC32 ハッシュ
│   ├── fsutil.h          # PNG 読み込み・バイナリファイル I/O
│   ├── BitStream.h       # ビット読み取りストリーム
│   ├── BitWriter.h       # ビット書き込みバッファ
│   └── externalImpl.cpp  # 外部ライブラリの実装インクルード
└── external/
    └── yy981/            # submodule: ユーティリティライブラリ
```

---

## ライセンス

[MIT License](LICENSE)