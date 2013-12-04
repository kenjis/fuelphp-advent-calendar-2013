
= FuelPHPのデータベースマイグレーションをPagoda Boxで使うときの注意


@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の3日目です。


今日は、FuelPHPのデータベースマイグレーションを@<href>{http::/pagodabox.com,Pagoda Box}で使う時にハマるポイントについて書きたいと思います。テーマがニッチ過ぎて、同じようなことで困っている人が他にもいるのか少し心配ではありますが、気にせず行きたいと思います。

== Pagoda Boxとは


まず、Pagoda Boxってなんや？というはなしを。


//image[11189458785_5f7e7c6b16_o][Pagoda Box (http::/pagodabox.com)]{
//}


@<href>{http::/pagodabox.com,Pagoda Box}は、LAMP環境を構築できるPHP専門のPaaSです。


PHPが使えるPaaSはいくつかありますが、Pagoda BoxはPHPに特化したPaaSなので、PHPエンジニアにはとてもなじみやすく、僕は結構気に入ってよく使っています。ちなみに、FuelPHPの公式サイトもPagoda Box上で動いているようです。


なお、Pagoda Boxについては、この間発売されたこの本に書きました。みなさん、よければ読んでみてください。


//image[9729281144_ef716fd9c9_n][『PHPエンジニア養成読本』]{
//}


『@<href>{http://www.amazon.co.jp/gp/product/4774159719/,PHPエンジニア養成読本 〔現場で役立つイマドキ開発ノウハウ満載! 〕（Software Design plus）}』


技術評論社


（著）新原 雅司、原田 康生、小山 哲志、田中 久輝、保科 一成、大村 創太郎、増永 玲


（編集）PHPエンジニア養読本編集部

== FuelPHPデータベースマイグレーションのおさらい


続いて、FuelPHPでのデータベースマイグレーションの手順を簡単におさらいしておきましょう。


公式サイトの説明は、@<href>{http://fuelphp.com/docs/general/migrations.html,http://fuelphp.com/docs/general/migrations.html}ですね。

=== oil generate model


@<tt>{$ oil generate model xxx}でModelクラスと、マイグレーション用のクラスが一気に作られます。僕は好みで、@<tt>{--crud}を使うことが多いです（というかcrudしか使いません）。

//emlist{
% php oil generate model post id:int name:varchar message:text created_at:datetime --crud --mysql-timestamp
    Creating model: /Users/omoon/Documents/www/speak_on_fuelphp/fuel/app/classes/model/post.php
    Creating migration: /Users/omoon/Documents/www/speak_on_fuelphp/fuel/app/migrations/001_create_posts.php
//}


で、つくられるModelクラス。

//emlist{
// fuel/app/classes/model/post.php
class Model_Post extends \Model_Crud
{
    protected static $_properties = array(
        'id',
        'name',
        'message',
        'created_at'
    );
    protected static $_mysql_timestamp = true;
    protected static $_table_name = 'posts';
}
//}


つくられるマイグレーションクラス

//emlist{
// fuel/app/migrations/001_create_posts.php
namespace Fuel\Migrations;
class Create_posts
{
    public function up()
    {
        \DBUtil::create_table('posts', array(
            'id' => array('constraint' => 11, 'type' => 'int'),
            'name' => array('constraint' => 255, 'type' => 'varchar'),
            'message' => array('type' => 'text'),
            'created_at' => array('type' => 'datetime'),

        ), array('id'));
    }
    public function down()
    {
        \DBUtil::drop_table('posts');
    }
}
//}

=== oil r migrate


で、@<tt>{$ oil r migrate}で該当テーブルがデータベースに作成されます。この際、

 * データベース上に @<tt>{migration} というテーブル
 * @<tt>{fuel/app/config/FUEL_ENV/migrations.php}というファイル（@<tt>{FUEL_ENV}は環境）



が作成され、マイグレーションの状況を管理するしくみになっています。

== 何が問題なのか


このしくみを使って、Pagoda Boxへアプリケーションをデプロイしたタイミングでデータベースの初期化まで一気にやってしまいたいのですが、以下の問題がありうまくいかないということがわかりました。

 * @<tt>{$ oil r migration}実行時に@<tt>{fuel/config/production/migrations.php}ファイルが生成されるので、@<tt>{fuel/config/production/}をwritableにする必要がある
 * Pagoda Boxでは、writableディレクトリは、ソースコードレポジトリとは別に、ネットワーク上にマウントされる
 * そのため、同じディレクトリにあるデータベースへの接続ファイル@<tt>{fuel/config/production/db.php}が消える
 * データベースへの接続ができなくなる


== 解決方法

 * @<tt>{fuel/app/config/pagoda/db.php}に別ファイルでPagoda Box設定ファイルを用意しておく
 * before_deployフックで、@<tt>{fuel/app/config/pagoda/db.php}ファイルを@<tt>{fuel/app/config/production/}配下へコピーする



という作戦で解決です。


参考までにBoxfile例。

//emlist{
global:
  env:
    - FUEL_ENV: production
db1:
  type: mysql
  name: speak
web1:
  shared_writable_dirs:
    - /fuel/app/cache
    - /fuel/app/logs
    - /fuel/app/tmp
    - /fuel/app/config/production # <- migrations.phpが作られるためwritableに
 document_root: public
 php_version: 5.4.14
 php_date_timezone: "Asia/Tokyo"
 php_extensions:
   - pdo_mysql
   - zip
 after_build:
   - "curl -s http://getcomposer.org/installer | php"
   - "php composer.phar install"
 before_deploy:
   # productionとは別のディレクトリにおいておいたdb.phpをコピー
   - "cp fuel/app/config/pagoda/db.php fuel/app/config/production/db.php"
   - "php oil r migrate"
//}


fuel/app/config/pagoda.db.phpはこうなります。

//emlist{
return array(
    'default' => array(
        'connection'  => array(
            'dsn'        => 'mysql:host='.$_SERVER['DB1_HOST'].';port='.$_SERVER['DB1_PORT'].';dbname='.$_SERVER['DB1_NAME'],
            'username'   => $_SERVER['DB1_USER'],
            'password'   => $_SERVER['DB1_PASS'],
        ),
    ),
);
//}


これで、@<tt>{$ git push pagoda}で一気にアプリケーションのデプロイからデータベースの初期化までできるようになります。


それでは、また。

//quote{
@<strong>{@omoon}


@TODO


Twitter: @<href>{https://twitter.com/omoon,@omoon}


Blog: @<href>{http://blog.omoon.org/,http://blog.omoon.org/}
//}
