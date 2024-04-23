# lcc - each Line Character Code filter

git でファイル単位で文字コードが混在している場合、ひとつの git コマン
ドの出力で行単位で文字コードが混在してしまいます。

このフィルタは行単位で文字コードを判定し、CP932 または UTF-8 に変換し
ます。

CP932 と UTF-8 のみの対応です。

Windows の DOS プロンプトやターミナルなどでは勝手に変換してくれるよう
になったので不要です。Windows で Emacs を使用していると必要になります。


## lccu - each Line Character Code filter to Utf-8

コマンド名をリネームし、最後の文字が u の場合、デフォルトで UTF-8 に変
換します。
