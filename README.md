# lcc - each Line Character Code filter

git でファイル単位で文字コードが混在している場合、ひとつの git コマン
ドの出力で行単位で文字コードが混在してしまう。それを修正するため、行単
位で文字コードを判定し、変換する。

## lccu - each Line Character Code filter to Utf-8

コマンド名をリネームし、最後の文字が u の場合、デフォルトで UTF-8 に変
換します。
