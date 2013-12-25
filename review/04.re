
= FuelPHPのエラーハンドリングがなんか今ひとつ物足りなかったのでなんとかしてみた話


@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}です。もう2013年ですね。早いですね。そうこうしているうちに2014年になります。なんとも恐ろしい。


思えば今年もFuelPHPでした。もうこの子しか愛せなさすぎて辛い。ポリアモリーを自称する割にはこういう所は変に一途だったりするのです（あとはまぁ眼鏡とか時計とかカバンとか）。


そういえば去年は何書いたかなぁ…と思ってFuelPHP Advent Calendar 2012を見に行ったら、自分の担当のリンクだけ「お探しのページは見つかりません」。というわけで本日は404のお話です。


FuelPHPのエラーハンドリングは何かと複雑です。便利機能が却って便利じゃなかったりとか、公式ドキュエントが@<tt>{index.php}書き換えたらイイヨ!!とか、もうなんかしったかめっちゃかな状況ですが、ざっと以下のような流れになっているみたいです。

 * リクエストの処理中にコントローラレベルでcatchされない例外が発生!!
 * HttpNotFoundException系列のみindex.phpにてキャッチ
 * ［index.php］config/routeに@<tt>{_404_}設定アレば、そちらで処理
 * ［index.php］無かったらそのまま例外横流し
 * 今度はエラーハンドラ（@<tt>{Error::exception_handler}）がキャッチ
 * ［エラーハンドラ］例外オブジェクトがhandleメソッド持ってたらそちらで処理
 * ［エラーハンドラ］handleが無くて本番環境なら、errors/productionをレンダする。
 * ［エラーハンドラ］handleが無くて本番でも無ければ、errors/php_fatal_errorをレンダする。



@<tt>{errors/production}とか@<tt>{errors/php_fatal_error}はcoreに入ってる方のView。同名ファイル作ればapp側で上書きできる。productionってのはあの「Oops!」ってやつで、php_fatal_errorってのはあの開発中に便利なバックトレースとかつけてくれる例外画面。


んで、ドキュメントを見るとなにやら@<tt>{HttpNotFoundException}って例外を投げるとエラー画面を描画してくれる、とかある。これはcoreに入ってるHttpNotFoundExceptionって例外クラスに例の@<tt>{handle}メソッドが実装されているからそうなるわけで、描画されるViewは@<tt>{views/404.php}になる。

== やりたいこと


CodeIgniterのエラーハンドリングでもそうなんだけど、実際アプリケーションの開発では1システム1エラー画面というわけには行かない。


PC版のリクエストならPC版の404を出したいし、SP版のリクエストならSP版の404を出したい。もっと言うならAPIのリクエストではJSON形式の404メッセージを送りたい（はず）。


CodeIgniter使ってた時にはエラーのView内で条件分岐して、リクエストの種別（PC/SP）毎にそれぞれ表示するView書き換えたりしてたので、それを応用しながら上手いこと出来ないかなぁとか考えてたら以下のような形になりました。

//emlist{
/**
 * Copyright (c) 2013 @mkkn_info
 * License: MIT 
 * http://opensource.org/licenses/mit-license.php
 */

class HttpNotFoundException extends \Fuel\Core\HttpNotFoundException
{
    public function response()
    {
        Fuel::$profiling = false;
        // デフォルト404出力の定義
        //$response = Response::forge(View::forge('404'), 404); // デフォルト
        $response = Request::forge("top/404")->execute()->response(); // デフォルト

        $req = Request::forge();
        $req->action = "404";
        try {
            $response = $req->execute()->response();
        } catch (Exception $e) {
            // 何もしない
        }
        $response->set_status(404);
        return $response;
    }
}
//}


各リクエストのURL形式からコントローラの検出まで可能であればそのコントローラに@<tt>{action_404}が存在しないかをチェックし、あればそれを出力する、なければデフォルトの404出力を返すというものです。


index.phpはいじらずにすみますが、coreのHttpNotFoundExceptionを上書きするのでbootstrapに以下のような記述が必要になります。

//emlist{
Autoloader::add_classes(array(
    // Add classes you want to override here
    // Example: 'View' => APPPATH.'classes/view.php',
    'HttpNotFoundException' => APPPATH.'classes/httpnotfoundexception.php' // 上記クラスを記述した場所
));
//}


クラス名が@<tt>{_}の全くない長い名前なので律儀にFuelPHPの命名規則に従う必要はないです。上記bootstrapの記述でしっかりパス指定さえ行えばどこに置いてもオートロードします。

== 解説

//emlist{
        Fuel::$profiling = false;
//}

//noindent
これを入れておかないとプロファイラが先に出力されて、HTMLがおかしくなる。端的に言うと文字が化ける。

//emlist{
        // デフォルト404出力の定義
        //$response = Response::forge(View::forge('404'), 404); // デフォルト
        $response = Request::forge("top/404")->execute()->response(); // デフォルト
//}


コメントアウトしてある上の行はViewファイルを直接指定するタイプのデフォルト404指定。コアの@<tt>{HttpNotFoundException}の挙動に近い形。その下の行は@<tt>{Uri}で直接指定するタイプ。@<tt>{config/router.php}の@<tt>{_404_}の書き方に近い形。
それぞれお好きな方をご利用ください。

//emlist{
        $req = Request::forge();
        $req->action = "404";
        try {
            $response = $req->execute()->response();
        } catch (Exception $e) {
            // 何もしない
        }
//}


例外の処理内部でリクエストを再生成して、actionだけ書き換えてからリトライする形。try catchしとかないと多重ループする気がする。もともと@<tt>{action_404}に対してのダメ元リトライなのでcatchしてもとくにすることはない。

== あとがき


Controllerごとにエラーハンドリングしたいなーって思いは結構前々からあったのですが、そこまでしっかりした開発をFuelPHPでやる機会もしばらく無く、ようやくこの機会に着手する事が出来ました。


正直@<tt>{views/404.php}が着地点なんだから、そこから@<tt>{Request::active()}なり@<tt>{Request::main()}なりでControllerのインスタンス引っ張ってきて@<tt>{action_404}のコールをトライしたら終わりじゃね？くらいの軽い気持ちだったのですが、よくよく処理を追いかけなおして見ると、バッチリ@<tt>{reset_request}なるメソッドが張り巡らされており、@<tt>{404.php}はおろか、HttpNotFoundExceptionからもControllerのインスタンスは取得できませんでした。まさかコントローラを再生成するはめにはなるとは…


FuelPHP的なコントローラは基本的に生成コスト低め、のはずなので問題無いとは思いますが、ファットなコントローラで生成コスト高め（特にbeforeがごちゃごちゃしてる…）の時には、処理負担的にあまりオススメできませんが、参考になれば幸いです。

//quote{
@<strong>{@mkkn_info}


@TODO


Twitter: @<href>{https://twitter.com/mkkn_info,@mkkn_info}


Blog: @<href>{http://mkkn.hatenablog.jp/,http://mkkn.hatenablog.jp/}
//}
