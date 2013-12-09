
= JavaScript側にPHP変数を簡単にまるごと渡す方法

ハイ、昨日のオレに引き続き@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の6日目です。


今回の内容もまたTwig絡みです。実は昨日の記事は、本日の記事の前準備になっていたのでした。

== JavaScript側にPHPのオブジェクトを渡したい


最近のWebアプリはUIのインタラクションが凝っていて、ブラウザ側のJavaScriptで色んな制御をすることも当たり前になってきました。jQueryや様々なjQueryプラグインを駆使して、ユーザに分かりやすく使いやすいサービスを提供することは、もはやウェブエンジニアとしては持っていて当然のスキルになっています。


そのようなUIを作っている際に、JavaScript側に動作パラメータの初期値を渡すのに値を1つ1つテンプレート記法で埋め込むのが面倒だったので、一発で渡せるTwig Extensionを作ったので紹介します。

== data_bind関数


アイデアとしては、見えないHTML要素を作成してそのテキストに値をJSON化して突っ込もうという、まぁ普通に思いつきそうなものです。でもこれがやってみると思ったより便利で。


extension本体はこんなコードです。

#@# lang: .syntax-highlight
//emlist{
<?php
// @TODO ライセンス明示
class Hoge_Twig_Extension extends Twig_Extension {
    …略…

    public function getFunctions()
    {
        return array(
            'data_bind' =>
                new Twig_Function_Method($this, 'dataBind'),
        );
    }

    public function dataBind($name, $val, $exclude = null)
    {
        if (is_object($val) && is_callable(array($val, 'to_array'))) {
            $val = $val->to_array();
        }
        if (! empty($exclude)) {
            if (is_string($exclude)) {
                $exclude = array($exclude);
            }
            foreach ($exclude as $key) {
                unset($val[$key]);
            }
        }
        $fmt = '<div id="data-%s" class="hide">%s</div>';
        return sprintf($fmt, $name, json_encode($val));
    } 
}
//}


div要素を不可視にするために、ここでは@<code>{hide}というclassを指定していますが、これはCSSで

#@# lang: .syntax-highlight
//emlist{
.hide {
  display: none;
}
//}

//noindent
的なものがあることを前提にしています。Bootstrapには含まれてますね。もちろん直接styleを書いてしまってもよいでしょう。


コントローラ側のアクションメソッドで以下のようにテンプレートに値を渡して、

#@# lang: .syntax-highlight
//emlist{
    function action_xxx()
    {
        …略…
        // $userinfoは情報が入ったObjectまたは連想配列
        $this->template->user = $userinfo;
    }
//}

//noindent
テンプレート側ではこう記述します。

//emlist{
{{ data_bind('user', user) }}
//}


JavaScript側でその値を使用するには、例えばjQueryだったら

#@# lang: .syntax-highlight
//emlist{
  var user = $.parseJSON($('#data-user').text());
//}

//noindent
と書くと、user変数にPHPで渡した値が入ります。

== パラメータの解説


data_bind関数は3つのパラメータを持ちます。

=== $name: 名前


HTML上で展開される名前です。'data-名前'がその要素のidになります。

=== $val: 変数


展開する変数です。Twigの変数になります。

=== $exclude: 排除するキー（省略可能）


変数を全部JS側に渡すのが楽とはいっても、ユーザ側に公開したくない内部プロパティが含まれているかもしれません。そういう場合には、第3引数にそのプロパティ名を渡すことであらかじめ削除した上で展開することができます。


単なる文字列として指定することもできますし、

//emlist{
{{ data_bind('user', user, 'password') }}
//}

//noindent
配列にして複数指定することもできます。

//emlist{
{{ data_bind('user', user, ['password', 'rank']) }}
//}


ということでお手軽 tips でした。明日のアドベントカレンダーは@@<href>{http://twitter.com/LandscapeSketch,LandscapeSketch}さんです。


== 【補足】FuelPHPのViewの自動エスケープについて

上記のエントリについて、PHPのjson_encode()関数は標準ではエスケープ処理は行わないのでXSS脆弱性があるのではないか、という指摘をいただきました。

=== json_encode()のエスケープオプション


確かにPHPのマニュアルには、各種文字にエスケープ対応するオプションが存在します。

//quote{
PHP: json_encode - Manual（@<href>{http://php.net/json_encode,http://php.net/json_encode}）
//}


この場合で言えば

#@# lang: .syntax-highlight
//emlist{
    return sprintf($fmt, $name, json_encode($val)); 
//}


を以下のようにエスケープオプションを追加するべきということです。

#@# lang: .syntax-highlight
//emlist{
    return sprintf($fmt, $name, json_encode($val, JSON_HEX_TAG |JSON_HEX_APOS | JSON_HEX_QUOT | JSON_HEX_AMP)); 
//}


ただこのコードを書いた時に自動的にエスケープ処理がかかることを確認していたので、どこでそれが行われるかは深く調べずに、json_encodeのオプションを省いたという経緯がありました。

=== 自動エスケープは\Fuel\Core\Viewの機能だった


その後@<href>{https://groups.google.com/forum/#!forum/fuelphp_jp,fuelphp.jpグループ}で@@<href>{http://twitter.com/kenji_s,kenji_s}さんに指摘されて、Parserパッケージの標準設定で'auto_encode'がtrueになっているおかげでテンプレートに渡される変数が自動でエスケープされていた事がわかりました。

//quote{
fuel/packages/parser/config/parser.php
//}


の以下の部分ですね。

#@# lang: .syntax-highlight
//emlist{
<?php
:
        'View_Twig' => array(
                'auto_encode' => true,
:
//}


このauto_encode設定は、\Fuel\Core\View のコンストラクタに$auto_filterとして渡され、結果的に\Fuel\Core\Security::clean()が呼び出されます。つまりTwig Extensionに渡される際にはすでにエスケープ済みになっていたわけですね。


PHP変数をJSONにしてJavaScriptに渡す仕組みは、別にFuelPHPでなくても使用できますので、その場合はXSSに注意してjson_encodeにオプションを随時追加して下さい。

=== JSON の埋め込み方の問題


他にもfuelphp.jpグループでは@@<href>{http://twitter.com/takayuki_h,takayuki_h}さんより、HTML要素にテキストとしてJSONを書き出すよりは、要素のdata-option属性として埋め込んだ方が良いのではないかとの指摘を受けました。

#@# lang: .syntax-highlight
//emlist{
  <div class="hidden">{"fuga":"hoge"}</div>
//}


ではなく

#@# lang: .syntax-highlight
//emlist{
  <div class="hidden" data-option='{"fuga":"hoge"}'></div>
//}


とせよということですね。ふむー、これはちょっと試してみたいと思います。

=== Twigのクラスが古かった


元記事とその前の記事で用いた以下のクラスはすでに古く、2.0でなくなる予定だと@@<href>{http://twitter.com/kenji_s,kenji_s}さんに指摘いただきました。

 * Twig_Filter_Function
 * Twig_Function_Method



これは気が付いてなかったので、元記事を修正します。

=== その他の反応への返事


はてブより。

//quote{
id:teppeis $nameもjson_encode()もエスケープが足りないです。危険。


@<href>{http://b.hatena.ne.jp/teppeis/20131207#bookmark-172246146,http://b.hatena.ne.jp/teppeis/20131207#bookmark-172246146}
//}


json_encodeについては上記に書いたとおり。$nameはテンプレートに直接記述されるので、そこに外部からの変数が渡される事態は、コード全体を見直したほうが良いレベルだと思うのですがどうでしょう?

//quote{
id:thujikun JSON形式のコードをJSの変数に直接代入する方が楽な気が。。。ひとつグローバル変数使うことにはなるけども。


@<href>{http://b.hatena.ne.jp/thujikun/20131208#bookmark-172246146,http://b.hatena.ne.jp/thujikun/20131208#bookmark-172246146}
//}


JavaScriptにテンプレートエンジンを通して変数展開を埋め込む方が、自分的にはあり得ないです。HTMLに埋め込み JS を直接記述することは現在は全くやっていません。

//quote{
id:fakechan PHPのレガシーっぷりに驚きを隠せない。というか、こういう場合はREST APIを作って「js側から」Ajaxでアクセスすればいいのでは。Ajaxのロードが終わるまでは「ロード中...」とかかぶせて。


@<href>{http://b.hatena.ne.jp/fakechan/20131208#bookmark-172246146,http://b.hatena.ne.jp/fakechan/20131208#bookmark-172246146}
//}


いやこれとPHPのレガシーは関係ないでしょ。PHPディスりたい病にかかっているようですね。何でもRESTでAjaxすれば良いやというのは、JS 側の処理を無駄に複雑にするだけではありませんか?


//quote{
@<strong>{@koyhoge}

@TODO

Twitter: @<href>{https://twitter.com/koyhoge,@koyhoge}

Blog: @<href>{http://d.hatena.ne.jp/koyhoge/,http://d.hatena.ne.jp/koyhoge/}
//}
