
= Authと他モデルにリレーションをつける

=== 要旨


Userテーブルに対してリレーションを付けたい。自分で作ったUserモデルであれば/project_name/fuel/app/class/model/に有るファイルを編集すればいい。Authパッケージを使いテーブルを作った場合packagesに有るファイルを編集してリレーションをつけて使っていた。packages内を直接編集するのは気持ち悪いしAuthパッケージをsubmoduleにしてアップデートすることが出来無くなる。解決するにはpackageを新しく作る。そこに作ったファイルにリレーションを記述すればよい。


packagesを拡張して使う記事は複数あるがリレーションを書いて拡張する方針の物は検索しても見つからなかった。そこでここではAuthを拡張してリレーションを付け加える方法を書く。私はPHPを使い始めて間もないので、この方針が正しいのか判別がつかない。Advent Calenderに投稿することでよりよい方法についてご助言をいただけたらと考えています。

=== 下準備


FuelPHPはoilで作ります。project_nameフォルダが作られます。WindowsのひとはDownloadして使って下さい。

//cmd{
# project_nameフォルダを作って、中にFuelPHPをダウンロードする
oil create project_name
//}

==== MySQLの準備


データベース（auth_test）と、管理ユーザーを作ります。

//cmd{
# MySQLにログインします
mysql -u root -p
# データベースを作る
create database auth_test;
# 管理ユーザーに権限を与える
# ユーザー名はdbuser,パスワードはlab1lab1
grant all on auth_test.* to dbuser@localhost identified by 'lab1lab1';
# 一旦ログアウトする
exit; 
# dbuserでログインする
mysql -u dbuser -p auth_test
# テーブルがないことを確認する
show tables;
//}

==== 設定ファイルの編集

 * @<href>{https://github.com/samui13/AuthExtend/commit/86557cc1160641afa30d4361f7bec89ad946bff2,Changeset}

/project_name/fuel/app/config/development/db.phpを編集します。データベースの名前と、使うユーザーとパスワードを入力します。

//emlist{
    return array(
        'default' => array(
            'connection'  => array(
                'dsn'        => 'mysql:host=localhost;dbname=auth_test',
                'username'   => 'dbuser',
                'password'   => 'lab1lab1',
            ),
        ),
    );
//}


/project_name/fuel/app/config/config.phpを編集します。always loadにORMとAuthを読み込むよう設定します。あとからMyAuthも読み込むよう設定をしなおします。

//emlist{
    'always_load'  => array(
        'packages'  => array(
            'orm',
            'auth',
            'myauth',
        ),
    ),
//}

==== Authの設定ファイルをコピーする

 * @<href>{https://github.com/samui13/AuthExtend/commit/6a7bf8256dc41c344e9307ea90447ec7819e6733,Changeset}


/projct_name/fuel/packages/auth/config/からauth.phpとormauth.phpを/project_name/fuel/app/config/にコピーします。

//cmd{
$ cp ./fuel/packages/auth/config/auth.php ./fuel/app/config/
$ cp ./fuel/packages/auth/config/ormauth.php ./fuel/app/config/
//}

auth.phpにはdriverとsaltを設定します。

//emlist{
    return array(
        'driver' => 'Ormauth',
        'verify_multiple_logins' => false,
        'salt' => 'ZomBie$22@OPS',
        'iterations' => 10000,
    );
//}


ormauth.phpにも同じようにlogin_hash_saltを設定します。

//emlist{
    'login_hash_salt' => 'ZomBie$22@OPS',
//}

=== ORMを編集する

==== 記事テーブルを作る

 * @<href>{https://github.com/samui13/AuthExtend/commit/6c1749f79f5c24312af6f4303fe7a2b7e4873ba6,Changeset}

Blogのような物を想定しているので、記事を保存するテーブルを作ります。テーブル名はarticles。id、title（タイトル）、comment（コメント）、user_id（どのユーザーの記事か）の4つのデータをもたせます。

//cmd{
# migrate用のファイルをつくる（このコマンドだけではデータベースにテーブルは作られていない）
oil generate model article title:varchar[255] comment:text user_id:int
# これを実行するとテーブルができる
oil refine migrate:up
//}

==== リレーションを付ける

 * @<href>{https://github.com/samui13/AuthExtend/commit/d022a15be6de908e3f96650cf24df7a5025689e5,Changeset}

articlesテーブルはUserに属しています。Usersテーブルは複数のarticlesを持ちます。

//emlist{
    protected static $_belong_to = 
        array('user'=>array(
            'model_to'=>'\MyAuth\Model\Auth_User',
            'key_from'=>'user_id',
            'key_to'=>'id',
            'cascade_save' => false,
            'cascade_delete' => true,
        ));
//}

==== Userテーブルを作る

 * @<href>{https://github.com/samui13/AuthExtend/commit/12ea5ef0167774dbfadd77f6aa41f9d6fbe165aa,Changeset}

Authパッケージにあるmigrateファイルからテーブルを作ります。

//emlist{
# テーブルを作ってもらいます。
oil refine migrate:current --packages=auth
//}

==== リレーションを付ける


Userテーブルは複数（has_many）のarticleを持ちます。これは後で書きます。

=== packagesを書き換える

==== ディレクトリ構成

 * @<href>{https://github.com/samui13/AuthExtend/commit/c31789892c2d5afc95518bc80a0e58397dbe7e3b,Changeset}

/project_name/fuel/packages/以下に次のようにファイルを作ります。

//emlist{
./myauth/
    |-- bootstrap.php
    `-- classes
    `-- model
        `-- auth
        `-- user.php
//}

==== ファイル


/auth/mode/auth/user.phpのファイルを/myauth/mode/auth/user.phpへコピーします。auth/user.phpには、functionや$_table_nameの変数を消しておきます。そして、継承元を\Orm\Modelから\Auth\Model\Auth_Userにかえます。

//emlist{
namespace MyAuth\Model;

class Auth_User extends \Auth\Model\Auth_User
{
    protected static $_has_many = array(
        'articles'=>array(
            'key_to'=>'user_id',
            'model_to'=>'Model_Article',
            'key_from'=>'id',
            'cascade_save' => false,
            'cascade_delete' => true,
        ),
    );
//}

=== Controllerを書き換える


Controllerを作り込むのは面倒なので/project_name/fuel/app/classes/controller/welcome.phpを編集します。action_test、action_create、action_loginの3つのメソッドを作ります。

==== Login Userを作る。


/project_name/上で、oil consoleを実行し、ユーザー名をsamui、パスワードをpassword、メールアドレスをss.to13@gmail.comで登録します。

//cmd{
Auth::create_user('samui','password','ss.to13@gmail.com');
//}


ユーザーIDは2または3をもつユーザーを作ることができます。

==== ログインページを作る

 * @<href>{https://github.com/samui13/AuthExtend/commit/2776f8cd421450501cc0ff07e3453e419be9c3b7,Changeset}

先程つくったユーザーで勝手にログインするメソッドを作ります。本来はFormを作ってユーザー入力させるべき部分です。今会は面倒なので作りません。

//emlist{
    public function action_login()
    {
        // Authのインスタンスを作る
        $auth = Auth::instance();
        // Authでログインできたら、
        if ($auth->login('samui',password'))
        {
            // welcome/viewに移動する
            Response::redirect('welcome/view');
        }
    }
//}

==== 記事を作る

 * @<href>{https://github.com/samui13/AuthExtend/commit/9cffd4a26593175048bcab904bb81aa76e891f34,Changeset}

articleに与えるデータを作るメソッドを書き加えます。

//emlist{
    public function action_create()
    {
        // Authでログイン出来てなかったら、
        if(! Auth::check())
        {
            // ログインさせる
            Response::redirect('welcome/login');
        }
        // ユーザーIDで、モデルからデータを取得
        $user = \Model\Auth_User::find(Auth::get_user_id()[1]);
        // $user = \MyAuth\Model\Auth_User::find(Auth::get_user_id()[1]);
        $regist = \Model_Article::forge(
            array(
                'title'=>'FirstBlog',
                'comment'=>'Comment!Comment!Comment!',
                'user_id'=>Auth::get_user_id()[1],
                'created_at'=>time(),
            )
        );
        $user->articles[] = $regist;
        $regist->save();
        $regist->user = $user;
        $user->save();
        Response::redirect('welcome/view');
    }
//}

==== 記事を表示する

 * @<href>{https://github.com/samui13/AuthExtend/commit/9e016739302d2d758e70059b2d8351c38051568e,Changeset}

正しくパッケージが読み込まれているか確認します。

//emlist{
    public function action_view()
    {
        // ユーザーデータ取得、
        $user = \Model\Auth_User::find(Auth::get_user_id()[1]); 
        // ユーザーデータのインスタンスの名前を表示
        echo get_class($user);  // MyAuth\Model\Auth_User
        // ユーザーからブログ記事を取得
        if (count($user->articles) > 0)
            echo get_class(($user->articles[0]));
        }
    }
//}

=== 動作確認


localhost/project_name/public/index.php/welcome/loginにアクセスします。すぐにlocalhost/project_name/public/in@<raw>{|latex|\n}dex.php/welcome/viewに飛されます。localhost/project_name/public/index.php/welcome/createに移動します。articlesが1つ追加されます。


インスタンスの名前が作ったものと一致すれば成功です。

=== マトメ


Authパッケージに僕が最初に不満に思ったことを改善する方法をまとめました。


Authパッケージを書き換えることでUserテーブルとのリレーションを作ると、Authパッケージの不具合が更新されると対処することが出来ません。Packagesの拡張を行うことで解決できることはFuelPHP界では自明なことのように扱われています。しかし、「auth リレーション」の検索ワードではこの解決を見付けることが出来ません。実際にサービスを作ってみるとUserテーブルと他のテーブルとの関係は直に付けたくなるものです。そこでAuthパッケージを使う所から一歩すんで初学者（フレームワークを使ったことがない人）が躓きそうであることについてまとめておきました。

=== 参考文献

==== packagesの拡張については

 * @<href>{http://fuelphp.jp/docs/1.7/general/packages.html,パッケージ - 概要 - FuelPHP ドキュメント}
 * @<href>{http://blog.livedoor.jp/erscape/archives/6841486.html,FuelPHPでパッケージを拡張する方法について}
 * @<href>{http://ryu-htn.hatenablog.com/entry/2013/06/26/133047,[FuelPHP]ネームスペースを上書きしてパッケージのクラスを書き換える}


==== Authについては

 * @<href>{http://moric.github.io/blog/2013/06/Fuelphp_auth/,[すごい広島]FuelPHPで認証機能を実装してみる!}


==== 独自に作ったユーザーテーブルと他のテーブルにリレーションを付ける

 * @<href>{http://teru2-bo2.blogspot.jp/2012/07/fuelphp_25.html,モデル間のリレショーンをためしてみた（FuelPHP）}


==== 今回つくったSource

 * @<href>{https://github.com/samui13/AuthExtend,AuthExtend}
 * @<href>{http://samui13.github.io/AuthExtend/,GitHubでの本文}

//quote{
@<strong>{samui_}

@TODO

Twitter: @<href>{https://twitter.com/samui__s,@samui__s}

Blog: @<href>{http://samui13.hatenablog.com/,http://samui13.hatenablog.com/}
//}
