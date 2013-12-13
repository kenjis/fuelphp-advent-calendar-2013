
= FuelPHPでChatWorkパッケージを使ってみる


@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013} 12日目です。@<href>{https://twitter.com/madmamor,@madmamor} が担当します。昨日は @<href>{https://twitter.com/chatii0079,@chatii0079} さんの「@<href>{http://chatii.net/articles/php/2013/12/fuelphp-composer.html,FuelPHP をもっと Composer で使う}」でした。  


 今日は、FuelPHPのChatWorkパッケージを紹介します。  


 ChatWorkのAPIは、昨月(2013年11月)末にプレビュー版として公開されました。@<br>{}
 @<href>{http://blog-ja.chatwork.com/2013/11/api-preview.html,http://blog-ja.chatwork.com/2013/11/api-preview.html}@<br>{}
 そこで早速、FuelPHPのバッケージとして実装してみました。@<br>{}
 @<href>{https://github.com/mp-php/fuel-packages-chatwork,https://github.com/mp-php/fuel-packages-chatwork}  


 ChatWorkパッケージのライセンスはMITライセンスです。@<br>{}
 @<href>{http://opensource.org/licenses/MIT,http://opensource.org/licenses/MIT}  


 では、準備と使い方の説明です。  

== 1. ChatWorkのAPIトークンを発行する


現在、ChatWorkのAPIはプレビュー版なので、利用の申請が必要です。


@<href>{http://blog-ja.chatwork.com/2013/11/api-preview.html,http://blog-ja.chatwork.com/2013/11/api-preview.html}


の"お申し込み方法"の通りに、APIの利用申請をします。後日、利用開始のメールが届きます。


利用開始のメールが届いたら、APIトークンを発行します。


@<href>{http://developer.chatwork.com/ja/authenticate.html,http://developer.chatwork.com/ja/authenticate.html}


の、"APIトークンの取得"の通りです。発行されたAPIトークンは後で使いますので、控えておいて下さい。  

== 2. FuelPHPとChatWorkパッケージのインストール


FuelPHPのインストールを済ませて、トップページが閲覧可能な状態にします。以下の手順はFuelPHP1.7.1で確認していますが、composer対応以降のバージョンであれば問題無いと思います。


 次に、composer.json の"require"に以下を追記します。

#@# lang: .brush:text
//emlist{
"mp-php/fuel-packages-chatwork": "dev-master"
//}


次に、composer update します。  

#@# lang: .brush:text
//emlist{
$ php composer.phar update
//}


 FuelPHPとChatWorkパッケージのインストールは以上です。尚、ChatWorkのAPIを使用する関係で、curlとOpenSSLを有効にしておいて下さい。  

== 3. ChatWorkパッケージの設定


fuel/packages/chatwork/config/chatwork.php を fuel/app/config/ にコピーして、ChatWorkのAPIトークンを設定します。

#@# lang: .brush:php
//emlist{
<?php

return array(
    'api_token' => 'ここにChatWorkのAPIトークンを設定します。',
);
//}


 次に、ChatWorkパッケージを有効にします。方法は二つあります。  


 各所でChatWorkパッケージを使う場合は@<br>{}
 fuel/app/config/config.php の"always_load.packages"に"chatwork"を追記します。  

#@# lang: .brush:php
//emlist{
'always_load'  => array(
    ...
    'packages'  => array(
        ...
        'chatwork',
        ...
//}


 局所的にChatWorkパッケージを使う場合は@<br>{}
 その場所(や、そのクラスのコンストラクタ等)でロードします。  

#@# lang: .brush:php
//emlist{
Package::load('chatwork');
//}


 これで、全ての準備が完了です。  

== 4. ChatWorkパッケージを使ってみる


自分の情報を取得してみます。@<br>{}
 @<href>{http://developer.chatwork.com/ja/endpoint_me.html#GET-me,http://developer.chatwork.com/ja/endpoint_me.html#GET-me}  

#@# lang: .brush:php
//emlist{
$response = Chatwork::get('/me');
Debug::dump($response);
//}


指定したルームに投稿してみます。@<br>{}
 @<href>{http://developer.chatwork.com/ja/endpoint_rooms.html#POST-rooms-room_id-messages,http://developer.chatwork.com/ja/endpoint_rooms.html#POST-rooms-room_id-messages}  

#@# lang: .brush:php
//emlist{
$response = Chatwork::post('/rooms/[ルームID]/messages', array('body' => '内容'));
Debug::dump($response);
//}


ルームIDは、URLに含まれる"rid"に続く数値です。"rid0123456789"であれば、ルームIDは"0123456789"になります。  


 どちらも簡単ですね。  


 今回はChatwork::get()メソッドとChatwork::post()メソッドを紹介しましたが、Chatwork::put()メソッドとChatwork::delete()メソッドも用意してあります。  

== 5. その他のAPIを使ってみる


現在、APIは大きく分けると  

 * /me
 * /rooms
 * /my
 * /contacts



の4種類で、それぞれ、更に細かくAPIが用意されています。@<br>{}
 @<href>{http://developer.chatwork.com/ja/endpoints.html,http://developer.chatwork.com/ja/endpoints.html}  


 どれを使うにしても

 * HTTPメソッドは何か
 * 必須パラメータは何か、任意パラメータにどんなパラメータがあるか



を確認すれば簡単に使えるので、ぜひ試してみてください。


 最後に、現在のChatWork APIの認証はトークン式のみなので、自分(トークンの持主)が使う前提になっています。パッと思いつく使い方としては、GitHubのHookから特定のルームにメッセージを投稿する。というような、通知の自動投稿でしょうか。タスクを管理する何か。も面白そうですね。  


 尚、今後、OAuth 2.0式の認証も提供予定とのことです。  


以上、ChatWorkパッケージの紹介でした。
