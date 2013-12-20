
= FuelPHPとMongoDBとTraceKitでJavaScriptのエラー情報を収集してみる


@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013} 18日目です。@<href>{https://twitter.com/madmamor,@madmamor}が担当します。

今日は、FuelPHPとMongoDBとTraceKitを使って、JavaScriptのエラー情報を監視、収集する方法を紹介します。@<br>{}

TraceKitはJavaScriptのエラーを簡単に監視できる、MITライセンスなJavaScriptライブラリです。

 * @<href>{https://github.com/occ/TraceKit,https://github.com/occ/TraceKit}


また、FuelPHPではMongoDBを簡単に扱えるので、それらを組み合わせることで、JavaScriptのエラーを容易に収集できるのでは。と思いつき、試してみました。


記事内のソースは、WTFPLライセンスとします。

 * @<href>{http://www.wtfpl.net/txt/copying/,http://www.wtfpl.net/txt/copying/}


以下、手順です。

== 1. 下準備


MongoDBとPECLモジュールのインストールを済ませておいて下さい。


MongoDB

 * @<href>{http://www.mongodb.org/,http://www.mongodb.org/}  


PECL :: Package :: mongo
 
 * @<href>{http://pecl.php.net/package/mongo,http://pecl.php.net/package/mongo}  


PHPからMongoDBが使用可能かは、phpinfo()で確認できます。


//image[shot][]{
//}


FuelPHPのインストールも済ませておいて下さい。トップページが見れる状態です。尚、当記事ではv1.7.1で確認しています。

== 2. TraceKitのインストール


@<href>{https://github.com/occ/TraceKit,https://github.com/occ/TraceKit}のtracekit.jsをダウンロードして、public/assets/jsに置きます。

== 3. FuelPHPの設定


config/db.php にMongoDB用の設定を追加します。以下、例です。

#@# lang: .brush:php
//emlist{
'mongo' => array(
    'tracekit' => array(
        'hostname'   => 'localhost',
        'port'       => '27017',
        'database'   => 'tracekit',
        'username'   => 'YOUR_USERNAME',
        'password'   => 'YOUR_PASSWORD',
    ),
),
//}

== 4. コントローラの作成


app/classes/controller/tracekit.phpを作成します。

#@# lang: .brush:php
//emlist{
<?php

/**
 * TraceKitが送信するエラー情報をMongoDBにInsertするコントローラ
 *
 * @author    Mamoru Otsuka http://madroom-project.blogspot.jp/
 * @copyright 2013 Mamoru Otsuka
 * @license   WTFPL License http://www.wtfpl.net/txt/copying/
 */
class Controller_Tracekit extends Controller
{

    /**
     * AjaxでPOSTされたエラー情報をMongoDBにInsertする
     */
    public function post_errors()
    {
        if (Input::is_ajax() and Security::check_token())
        {
            $input = Input::post();
            unset($input[Config::get('security.csrf_token_key')]);

            $mongodb = Mongo_Db::instance('tracekit');
            $mongodb->insert('errors', $input);
        }
    }

}
//}

== 5. viewの修正


app/views/welcome/index.php を修正します。

#@# lang: .brush:html
//emlist{
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>FuelPHPとMongoDBとTraceKitでJavaScriptのエラー情報を収集してみる</title>
    <!-- エラー情報の送信にjQueryを使います -->
    <?php echo Asset::js('http://code.jquery.com/jquery-1.10.2.min.js'); ?>
    <!-- tracekit.jsです -->
    <?php echo Asset::js('tracekit.js'); ?>
    <!-- jQueryでPOSTする際のトークン生成に使います -->
    <?php echo Security::js_fetch_token(); ?>
    <script>
        // TraceKitでエラーを購読します
        TraceKit.report.subscribe(function myLogger(errorReport) {
            // トークンをセットします
            errorReport.<?php echo Config::get('security.csrf_token_key'); ?> = fuel_csrf_token();
            // エラー情報をPOSTします
            $.post('<?php echo Uri::create('tracekit/errors'); ?>', errorReport);
        });
        // 意図的にエラーを発生させてみます
        throw new Error('oops');
    </script>
</head>
<body>
    <h1>FuelPHPとMongoDBとTraceKitでJavaScriptのエラー情報を収集してみる</h1>
</body>
</html>
//}

== 6. 動作の確認


プラウザからトップページにアクセスして、MongoDBを確認します。MongoDBに以下のようなエラーデータが入っていれば正しく動いています。

#@# lang: .brush:text
//emlist{
{
   "_id": ObjectId("52aef1afece0b97316058000"),
   "mode": "onerror",
   "message": "Uncaught Error: oops",
   "url": "http://192.168.33.10/fuel/",
   "stack": [
     {
       "url": "http://192.168.33.10/fuel/",
       "line": "41",
       "func": "myLogger",
       "context": [
         "\t\t\terrorReport.fuel_csrf_token = fuel_csrf_token();",
         "\t\t\t// エラー情報をPOSTします",
         "\t\t\t$.post('http://192.168.33.10/fuel/tracekit/errors', errorReport);",
         "\t\t});",
         "\t\t// 意図的にエラーを発生させてみます",
         "\t\tthrow new Error('oops');",
         "\t</script>",
         "</head>",
         "<body>",
         "\t<h1>FuelPHPとMongoDBとTraceKitでJavaScriptのエラー情報を収集してみる</h1>",
         "</body>" 
      ] 
    } 
  ],
   "useragent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/@<raw>{|latex|\n}31.0.1650.63 Safari/537.36" 
}
//}


どんな情報が含まれているか、簡単に確認してみましょう。   

 * message ... エラーメッセージです。普段、コンソールに出るヤツです。
 * url ... エラーが発生したURLです。
 * stack.line ... エラーが発生した行、のように見えますが、ズレています。
 * stack.func ... JavaScript側の送信関数名です。
 * stack.context ... 発生した行と、前後5行ずつのソースです。
 * useragent ... ユーザエージェントです。



注意: stack.contextにminifyされたJavaScriptが含まれると、相当な量になってしまいます。

== 7. まとめ


FuelPHPとMongoDBとTraceKitを組み合わせると、JavaScriptのエラーを簡単に保存できました。ブラウザで発生するエラーも、こういった方法で把握して、改善していきたいものです。

//quote{
@<strong>{@madmamor}

Developer & Bassist on TAMACENTER OUTSiDERS.

Twitter: @<href>{https://twitter.com/madmamor,@madmamor}

Blog: @<href>{http://madroom-project.blogspot.jp/,http://madroom-project.blogspot.jp/}
//}
