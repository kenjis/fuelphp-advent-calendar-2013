= FuelPHPでTwig Extension

4日目の@<href>{https://twitter.com/mkkn_info,@mkkn_info}さんの「Fuelphpのエラーハンドリングがなんか今ひとつ物足りなかったのでなんとかしてみた話 - どうにもならない日々@mkkn」に引き続き、@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の5日目です。

ここ数年はアドベントカレンダーの時にしか技術的な内容を書いていない気がするのが恐ろしいところですが、気にせずいきましょう。

== FuelPHPのParserパッケージ

FuelPHPは、基本的にはビューに生のPHPスクリプトを使うことになっていますが、標準バンドルされているParserパッケージを用いることで、様々なテンプレートエンジンを用いることができます。現在サポートされているエンジンは以下の通り。

 * @<href>{http://twig.sensiolabs.org/,Twig}
 * @<href>{http://mustache.github.io/,Mustache}
 * @<href>{http://pecl.php.net/package/markdown,Markdown}
 * @<href>{http://dwoo.org/,Dwoo}
 * @<href>{http://jade-lang.com/,Jade}
 * @<href>{http://haml.info/,Haml}
 * @<href>{https://github.com/arnaud-lb/MtHaml,MtHaml}
 * @<href>{http://www.smarty.net/,Smarty}
 * @<href>{http://phptal.org/,PHPTAL}

このうち自分ではTwigを愛用しています。何か機能を追加するにも簡単にできるところが良いですね。

== Parserパッケージが標準で用意してくれるFuelPHP向けExtension

ParserパッケージでTwigを使用すると、Uri、Config、Form、Input、Html、Assetなどの便利そうなFuel coreのメソッドを、あらかじめTwig Extensionとしてロードしてくれます。これを行っているのは

//quote{
fuel/packages/parser/classes/twig/fuel/extension.php
//}

//noindent
にある\Parser\Twig_Fuel_Extensionクラスで、これ自体も標準的なTwig Extensionです。

これのおかげで、例えば

#@# lang: .syntax-highlight
//emlist{
Asset::js('hogehoge.js');
//}

//noindent
を呼びたい場所では

#@# lang: .syntax-highlight
//emlist{
{{ asset_js('hogehoge.js') }}
//}

//noindent
と書くことができるわけです。

== アプリ独自のTwig Extensionを使う

とはいえ、ただ単にTwigを使ってHTMLテンプレートを書くだけではなく、アプリケーション独自のTwig Extensionをがしがし登録して使いこなしてこそTwigの便利さが際立つというもの。早速やってみましょう。

独自のTwig Extensionを登録するには、まずTwig_Extensionクラスを継承したクラスを作ります。クラス名は他とぶつからなければ何でも良いですが、ここではHogeアプリ向けにHoge_Twig_Extensionという名前にすることにしましょう。FuelPHPのファイル名規則に則り以下の場所に作ります。

//quote{
fuel/app/classes/hoge/twig/extension.php
//}

中身はこんな感じ。

#@# lang: .syntax-highlight
//emlist{
<?php
/*
 * Copyright (c) 2013 KOYAMA Tetsuji
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 */

class Hoge_Twig_Extension extends Twig_Extension
{
    /**
     * Gets the name of the extension.
     *
     * @return  string
     */
    public function getName()
    {
        return 'hoge';
    }

    /**
     * Sets up all of the functions this extension makes available.
     *
     * @return  array
     */
    public function getFunctions()
    {
        return array(
            new Twig_SimpleFunction('swap_empty', array($this, 'swapEmpty')),
        );
    }

    /**
     * Sets up all of the filters this extension makes available.
     *
     * @return  array
     */
    public function getFilters()
    {
        return array(
            new Twig_SimpleFilter('json', 'json_encode'),
        );
    }

    public function swapEmpty($value)
    {
        return empty($value)? '-' : $value;
    } 
} 
//}

ここではTwigの関数とフィルターを1つずつ登録しています。

: swap_empty関数
    もし引数がempty()で真だったら「-」を出力、そうでなければそのまま。
: jsonフィルター
    引数をPHPのjson_encodeに渡した結果を出力。

テンプレート上では以下のように使います。

#@# lang: .syntax-highlight
//emlist{
{# 変数の設定、本来はPHP側から渡される #}
{% set foo = 0 %}
{% set bar = {'fuga': 'hoge', 'move': 'puge'} %}

{{ swap_empty(foo) }}
{{ bar|json }}
//}

ただファイルを置いただけではParserパッケージはそのExtensionの存在を知らないので、Parserのconfigを通して教えてやります。FuelPHPのConfigは大変に賢くて、追加・変更したい部分だけapp以下に書けば良いので、

//quote{
fuel/app/config/parser.php
//}

//noindent
に以下の内容を記述します。

#@# lang: .syntax-highlight
//emlist{
<?php
return array(
    'View_Twig' => array(
        'extensions' => array(
            'Hoge_Twig_Extension',
        ),
    ),
); 
//}

Twigは非常に柔軟性の高いテンプレートエンジンで、上記で紹介した関数・フィルターの他にも

 * グローバル変数
 * タグ
 * オペレータ
 * テスト

などを独自に拡張できます。詳しく知りたい方は、@<href>{http://twig.sensiolabs.org/doc/advanced.html,Extending Twig}を読むとよいでしょう。

明日のアドベントカレンダーも引き続き私ですw

//quote{
@<strong>{@koyhoge}

2014年からはフリーになる予定は未定のPHP大好きっこです、テへ☆

Twitter: @<href>{https://twitter.com/koyhoge,@koyhoge}

Blog: @<href>{http://d.hatena.ne.jp/koyhoge/,http://d.hatena.ne.jp/koyhoge/}
//}
