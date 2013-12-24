
= 本当は怖いFuelPHP 1.6までのRestコントローラ post


@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の24日目です。


FuelPHP 1.6までには、Restコントローラを使うとXSS脆弱性を作り込みやすい隠れ機能（アンドキュメントな機能）がありました。


具体的には、公式ドキュメントのサンプルコードをそのまま実行すれば、XSSが可能でした。


ただし、FuelPHP 1.7ではバグのためエラーが発生しXSSは成立せず、1.7.1で修正されました。


なお、この問題を重視しているのは世界中で私1人かも知れませんので、実際に影響があるユーザは少ないのかも知れません。本家は@<href>{http://fuelphp.com/security-advisories,セキュリティ勧告}は出しませんでした（@<href>{https://github.com/fuel/fuel/blob/1.7/master/CHANGELOG.md,Changelog}に仕様変更の記述はあります）し、@<href>{https://groups.google.com/forum/#!forum/fuelphp_jp,fuelphp.jp Googleグループ}で告知しましたが、特に反応はありませんでした。


ただし、まだ知らないユーザの方もいるかも知れませんし、今後の参考のためにも、どのような問題があったのかまとめておきます。

== Restコントローラとは？


Restコントローラとは、RESTfuelなAPIを簡単に作成するためのコントローラです。以下は@<href>{http://fuelphp.jp/docs/1.7/general/controllers/rest.html,公式ドキュメント}からのサンプルコードです。

//emlist{
class Controller_Test extends Controller_Rest
{
    public function get_list()
    {
        return $this->response(array(
            'foo' => Input::get('foo'),
            'baz' => array(
                1, 50, 219
            ),
            'empty' => null
        ));
    }
}
//}


http://localhost/fuel/以下がFuelPHPだとして、


ブラウザから、


http://localhost/fuel/test/list.json


にアクセスすれば、以下のようにJSONでの結果が返ります。

//emlist{
{"foo":null,"baz":[1,50,219],"empty":null}
//}


http://localhost/fuel/test/list.xml


にアクセスすれば、以下のようにXMLでの結果が返ります。

//emlist{
<?xml version="1.0" encoding="utf-8"?>
<xml><foo/><baz><item>1</item><item>50</item><item>219</item></baz><empty/></xml>
//}


このようにアクセスされるURLなどにより動的に出力を変更する機能を持っています。

== XSS脆弱性の解説


それでは、


http://localhost/fuel/test/list.html


にアクセスしたらどうでしょうか？結果は以下のようになりました。

//emlist{
array(3) {
  ["foo"]=>
  string(0) ""
  ["baz"]=>
  array(3) {
    [0]=>
    int(1)
    [1]=>
    int(50)
    [2]=>
    int(219)
  }
  ["empty"]=>
  string(0) ""
}
//}


何故か、var_dump()した結果が表示されました。


ということで、Firefoxから以下のURLにアクセスすれば、めでたく警告ダイアログが表示されます。


http://localhost/fuel/test/list.html?foo=%3Cscript%3Ealert%28document.cookie%29%3C/script%3E


それなら、エスケープすればいいのでは？と普通は考えます。

//emlist{
            'foo' => Input::get('foo'),
//}


上記のInput::get('foo')は$_GET['foo']を返すFuelPHPのメソッドです。これを以下のようにエスケープして渡せばいいと考えるでしょう。e()は、FuelPHPでのhtmlentities()関数みたいなものです。

//emlist{
            'foo' => e(Input::get('foo')),
//}


でもダメです。実際に試してみると、結果は変わりません。

== どこに問題があったのか？


Fuel\Core\Responseクラスの__toString()メソッドに問題がありました。

//emlist{
    /**
     * Returns the body as a string.
     *
     * @return  string
     */
    public function __toString()
    {
        // special treatment for array's
        if (is_array($this->body))
        {
            // this var_dump() is here intentionally !
            ob_start();
            var_dump($this->body);
            $this->body = html_entity_decode(ob_get_clean());
        }

        return (string) $this->body;
    }
//}


配列の場合は、var_dump()して、しかもhtml_entity_decode()して返されています。ここが問題の所在です。Restコントローラから配列を返す場合は、ここでvar_dump()されるというわけです。


さきほどのサンプルコードでは、Restコントローラで配列を作成しているため、配列であることがすぐにわかりますが、ORMなどからも結果が配列で返ることもありますので注意してください。


今となっては何故こんなコードがあったのか正確にはわかりませんが、デバッグ用のためのもののように思われます。デバッグ用コードがデフォルトで有効になっており、それが脆弱性につながるという典型的なパターンのように思われます。

== 対策


FuelPHP 1.7.1（＝現在の1.7/masterブランチ）にアップデートすれば問題は修正されています。


http://localhost/fuel/test/list.html?foo=%3Cscript%3Ealert%28document.cookie%29%3C/script%3E にアクセスしても本番環境以外では、以下のようにメッセージとJSONでの結果が表示され、

//emlist{
The requested REST method returned an array:

{ "foo": "\u003Cscript\u003Ealert(document.cookie)\u003C\/script\u003E", "baz": [ 1, 50, 219 ], "empty": null }
//}


本番環境では406 Not Acceptableが返ります。


アップデートが無理な場合は、Restコントローラで出力フォーマットをjsonやxml（html以外）に固定すればいいでしょう。

//emlist{
    protected $format = 'json';
//}


こう指定することで、URLの拡張子から出力フォーマットを動的に決定することはなくなり、出力フォーマットが固定されます。


というか、FuelPHPのドキュメントには

//quote{
結果のフォーマットをRESTコントローラ内でハードコードすることはバッドプラクティスであることに注意してください。
//}


と書いてあるんですが、私自身はなんでバッドプラクティスなのかよくわかりません。なので、必ず、フォーマットは固定してます。バッドプラクティスである理由がわかる方がいましたら、是非、お教え願いたいです。


ああ、それから、JSONを返すWeb APIなどは、そもそもブラウザからの直接のアクセスは禁止した方がいいでしょうね。JSONを返すWeb APIの作り方は、『@<href>{http://2nd.php-recipe.com/,PHP逆引きレシピ 第2版}』で解説されていますので、興味のある方はご覧ください（宣伝）。あの徳丸先生も

//quote{
この第2版は本当に素晴らしい。PHPセキュリティの最新動向をよく把握して、具体的なレシピに落とし込んでいます。すべてのPHP開発者にお勧めします。（…略…）冒頭に書いたように、この第2版は素晴らしいです。「もうペチパーは緩いなんて言わせない」と叫びたくなるほどのインパクトがあります。@<br>{}
 @<href>{http://blog.tokumaru.org/2013/12/php12sql.html,http://blog.tokumaru.org/2013/12/php12sql.html}
//}


と絶賛されてますので、一家に一冊あって損はないと思います（宣伝）。


あと、FuelPHP 1.7.1からRestコントローラからのJSON出力もそうですが、FormatクラスのJSON出力でのエスケープ（json_encode()関数の第2引数のオプション指定）がより安全なものに変更されています。1.7以前はオプション指定はありませんでした。

== 最後に


どんなフレームワークも完璧ではないと考えた方が現実的です。今まで、脆弱性が修正されたことがないメジャーなフレームワークというのはないと思います。そのため、フレームワークのソースを読み、実装が本当に安全か確認することが有効です。


FuelPHPは規模的にもまだソースが読めるっぽい分量だと思いますし、読みやすいと思いますので、みなさん、がんがんソースを読んで、バグなどがあれば修正するPull Requestを本家に送るなり、とりあえず、Googleグループで相談してみるとよいと思います。


ただし、セキュリティ上の問題を発見した場合は、いきなりPull Requestを送ったり、Googleグループに投稿するなど、いきなり内容を公開してはいけません。これは、脆弱性情報がいきなり公開されても、即座には対応できないため、ユーザがより危険な状態に置かれてしまうからです。脆弱性情報はその対策ととも公開されるべきものです。


本家へセキュリティ上の問題を報告する場合は、@<href>{http://fuelphp.com/contact,http://fuelphp.com/contact}のコンタクトフォームからしてください。


明日は、@samui_さんの「Authのユーザーモデルとほかモデルへのリレーションを作る」です。お楽しみに！
