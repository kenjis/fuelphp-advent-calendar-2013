
= FuelPHP 5分でAPIを実装するチュートリアル（スクリーンキャストあり）





@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013} の 19 日目です。@<br>{}


APIを作る機会が増えていますよね。スマホアプリから叩いたり、JavaScriptのフレームワークから叩いたりと。


あなたも私も、いきなり誰かに「私、APIが叩きたいの@<uchar>{2665}」と言われるかも知れません。


ということで、いつそうなってもいいように、FuelPHPでAPIを実装する流れをおさらいしておきましょう。@<br>{}


FuelPHPには@<tt>{Controller_Rest}というものが用意されていて、APIを作るのがとても簡単なんですよー、とはよく言われるところですが、実際にどれぐらい簡単なのかやってみました。

 * Building an api on FuelPHP in 5 minutes @<href>{http://www.youtube.com/watch?v=Uin3hh0ldgM,http://www.youtube.com/watch?v=Uin3hh0ldgM}

結果は、FuelPHPのインストールも含めて、4分44秒で実装できました。


以下、詳しく説明します。

== 前提条件


今回は、下記環境での実装例となります。

 * PHP 5.4（Built-in web server、Short array syntax を使っています）
 * FuelPHP 1.7.1
 * データベースにMySQLを利用
 * MySQLのhostはlocalhost、portは3306、ユーザ、パスワードともに@<tt>{root}、データベース名は@<tt>{fuel_dev}（FuelPHP インストール時の@<tt>{development}環境でのデフォルト設定です）



では、はじめましょう。

== データベースの準備


あらかじめ、@<tt>{fuel_dev}という名前のデータベースを作っておきます。

//cmd{
$ echo 'create database fuel_dev' | mysql -u root -proot
//}

== FuelPHP のインストール


@<tt>{oil}コマンドでさくっと行きましょう。参考）@<href>{http://fuelphp.com/docs/,http://fuelphp.com/docs/}

//cmd{
$ curl get.fuelphp.com/oil | sh
$ cd Sites/
$ oil create api
//}


できたら、そのディレクトリへ移動。

//cmd{
$ cd api
//}


一応念のため、動くか確認しておきましょう。ビルトインサーバ使います。

//cmd{
$ php -S localhost:8000 -t public
//}


はいこんにちは。


//image[11446914955_4d5c8c6d3f_z][]{
//}

== 何を作るか


今回は、簡単なメモみたいなのを作りましょう。メモを記録したり、取得したりするだけのもの。

 * http://localhost:8000/memo/write （POSTメソッドで）
 * http://localhost:8000/memo/read （GETメソッドで、JSONを返す）



という２つのURLを用意して、それぞれ、メモの投稿と取得をさせたいと思います。


では、進みましょう。

== oil generateでmodel作成


@<tt>{oil generate}でモデル作ります。

//cmd{
$ php oil generate model memo memo:varchar created_at:timestamp updated_at:timestamp --mysql-timestamp --crud@<raw>{|latex|\n} --created-at --updated-at
//}


僕の好みで、@<tt>{--crud --mysql-timestamp --created-at --updated-at}オプションをつけてます。

//cmd{
    Creating model: /Users/omoon/Documents/api/fuel/app/classes/model/memo.php
    Creating migration: /Users/omoon/Documents/api/fuel/app/migrations/001_create_memos.php
//}


とメッセージが表示されて、modelと同時にマイグレーションファイルも作成されていることが分かります。


忘れないうちに、そのまま@<tt>{oil r migrate}して、テーブルを作成してしまいましょう。

//cmd{
$ php oil r migrate
Performed migrations for app:default:
001_create_memos
//}

== route.php の編集


続いて、@<tt>{fuel/app/config/route.php}を下記のように編集します。

//emlist{
// fuel/app/config/route.php
return array(
    '_root_'  => 'welcome/index',  // The default route
    '_404_'   => 'welcome/404',    // The main 404 route

    'memo/read'  => 'memo/read',
    'memo/write' => 'memo/write',
);
//}


これで、

 * http://localhost:8000/memo/write
 * http://localhost:8000/memo/read



この２つのURLへアクセスできるようになります。

== oil generateでcontroller作成


上記で設定したルーティングに合わせてコントローラを作りましょう。


FuelPHP では、あらかじめ用意されている@<href>{http://fuelphp.com/docs/general/controllers/rest.html,Controller_Rest}を継承して作ります。ここがキモですね。


ここも@<tt>{oil generate}でさくっと行きたいところですが、ざっと見た感じでは、Controller_Rest用のオプションなどがあまり用意されていないようでしたので、ある程度まで作ってちょこっと修正する方針で行きます。

//cmd{
$ php oil generate controller memo write read --extends=Controller_Rest
//}


これで、@<tt>{fuel/app/classes/controller/memo.php}に下記のようなファイルができあがりますので、

//emlist{
class Controller_Memo extends Controller_Rest
{

    public function action_read()
    {
        $data["subnav"] = array('read'=> 'active' );
        $this->template->title = 'Memo @<uchar>{00BB} Read';
        $this->template->content = View::forge('memo/read', $data);
    }

    public function action_write()
    {
        $data["subnav"] = array('write'=> 'active' );
        $this->template->title = 'Memo @<uchar>{00BB} Write';
        $this->template->content = View::forge('memo/write', $data);
    }

}
//}


このように編集します。

//emlist{
class Controller_Memo extends Controller_Rest
{

    protected $format = 'json';

    public function get_read()
    {
        $memos = Model_Memo::find_all();
        $this->response($memos);
    }

    public function post_write()
    {
        $memo = Input::post('memo');
        Model_Memo::forge(['memo' => $memo])->save();
    }

}
//}


ポイントは、

 * @<tt>{protected $format = 'json';}でデフォルトフォーマットをJSONに
 * @<tt>{public function get_read()}で@<tt>{read}をGETメソッドで受け付けるように
 * @<tt>{public function post_write()}で@<tt>{write}をPOSTメソッドで受け付けるように



各メソッドでは、今回は単純に@<tt>{Model_Memo::find_all()}で取得、@<tt>{Model_Memo::forge()->save()}で保存を行っています。


さ、これでできたはずです。

== 動作確認


curlコマンドでちゃんと動くか確認します。


POSTで書き込み。

//cmd{
$ curl http://localhost:8000/memo/write -d "memo=test1"
$ curl http://localhost:8000/memo/write -d "memo=test2"
$ curl http://localhost:8000/memo/write -d "memo=test3"
//}


GETで取得。

//cmd{
$ curl http://localhost:8000/memo/read
[{"id":"1","memo":"test1","created_at":"2013-12-19 19:48:08","updated_at":"2013-12-19 19:48:08"},{"id":"2","memo"@<raw>{|latex|\n}:"test2","created_at":"2013-12-19 19:48:12","updated_at":"2013-12-19 19:48:12"},{"id":"3","memo":"test3","created@<raw>{|latex|\n}_at":"2013-12-19 19:48:15","updated_at":"2013-12-19 19:48:15"}]
//}


無事、JSONが返却されました。

== 最後に


噂通り、FuelPHPでのAPI実装はとてもお手軽でした。みなさんも一度やってみておくと何かの時にアワアワしなくていいのではないでしょうか。


ところで、ゴリゴリ記事を書いて、必死でスクリーンキャストまで撮って、公開ボタンを押そうとしたまさにその時、@<href>{https://twitter.com/kenji_s,@kenji_s}さんが、過去に同様のことをなさっていることを発見してしまいました。

 * @<href>{http://d.hatena.ne.jp/Kenji_s/20130121/fuel_web_api_5_min,FuelPHPを使って5分でWeb APIを作成する}



決してパクったわけではありません（信じてー）が、もうちょっと過去記事などをしっかり見ておくべきでした。。大変失礼しました。


ただ（言い訳みたいですが）、@kenji_sさんのバージョンは1.4で今回は1.7.1、また、実装しているAPIや手法も少し異なりますので、実装方法の異なるバージョンとして見ていただければ幸いです。汗。。





では、また。

//quote{
@<strong>{omoon}

40代、縄文系。大阪を拠点にPHPなどを使って仕事をしています。Kansai PHP Users Groupスタッフ。

Twitter: @<href>{https://twitter.com/omoon,@omoon}

Blog: @<href>{http://blog.omoon.org/,http://blog.omoon.org/}
//}
