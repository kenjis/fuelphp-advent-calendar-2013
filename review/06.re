
= @<href>{/koyhoge/20131206/twig_databind,JavaScript側にPHP変数を簡単にまるごと渡す方法 #FuelPHPAdvent2013}


@<href>{/koyhoge/searchdiary?word=%2A%5Bphp%5D,php}


ハイ、昨日のオレに引き続き@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の6日目です。


今回の内容もまたTwig絡みです。実は昨日の記事は、本日の記事の前準備になっていたのでした。

== JavaScript側にPHPのオブジェクトを渡したい


最近のWebアプリはUIのインタラクションが凝っていて、ブラウザ側のJavaScriptで色んな制御をすることも当たり前になってきました。jQueryや様々なjQueryプラグインを駆使して、ユーザに分かりやすく使いやすいサービスを提供することは、もはやウェブエンジニアとしては持っていて当然のスキルになっています。


そのようなUIを作っている際に、JavaScript側に動作パラメータの初期値を渡すのに値を一つ一つテンプレート記法で埋め込むのが面倒だったので、一発で渡せるTwig Extensionを作ったので紹介します。

== data_bind関数


アイデアとしては、見えないHTML要素を作成してそのテキストに値をJSON化して突っ込もうという、まぁ普通に思いつきそうなものです。でもこれがやってみると思ったより便利で。


extension本体はこんなコードです。

#@# lang: .syntax-highlight
//emlist{
<?php
class Hoge_Twig_Extension extends Twig_Extension {
  :
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
        if (!empty($exclude)) {
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


div要素を不可視にするために、ここでは'hide'というclassを指定していますが、これはCSSで

#@# lang: .syntax-highlight
//emlist{
.hide {
  display: none;
}
//}


的なものがあることを前提にしています。Bootstrapには含まれてますね。もちろん直接styleを書いてしまってもよいでしょう。


コントローラ側のアクションメソッドで以下のようにテンプレートに値を渡して、

#@# lang: .syntax-highlight
//emlist{
<?php
 :
  function action_xxx()
  {
    :
    // $userinfo は情報が入ったObjectまたは連想配列
   $this->template->user = $userinfo;
  }
//}


テンプレート側ではこう記述します。

//emlist{
{{ data_bind('user', user) }}
//}


JavaScript側でその値を使用するには、例えばjQueryだったら

#@# lang: .syntax-highlight
//emlist{
  var user = $.parseJSON($('#data-user').text());
//}


と書くと、user変数にPHPで渡した値が入ります。

== パラメータの解説


data_bind 関数は3つのパラメータを持ちます。

=== $name: 名前


HTML上で展開される名前です。'data-名前' がその要素のidになります。

=== $val: 変数


展開する変数です。Twigの変数になります。

=== $exclude: 排除するキー (省略可能)


変数を全部JS側に渡すのが楽とはいっても、ユーザ側に公開したくない内部プロパティが含まれているかもしれません。そういう場合には、第3引数にそのプロパティ名を渡すことであらかじめ削除した上で展開することができます。


単なる文字列として指定することもできますし、

//emlist{
{{ data_bind('user', user, 'password') }}
//}


配列にして複数指定することもできます。

//emlist{
{{ data_bind('user', user, ['password', 'rank']) }}
//}


ということでお手軽 tips でした。明日のアドベントカレンダーは@@<href>{http://twitter.com/LandscapeSketch,LandscapeSketch}さんです。
