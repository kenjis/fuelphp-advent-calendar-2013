= レンタルサーバーXREA/CORESERVERでFuelPHPを使う（実践編）

この記事は@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の17日目の記事として公開します。

先々月に「@<href>{http://d.hatena.ne.jp/suno88/20131006/1381040481,レンタルサーバーXREA/CORESERVERでFuelPHPを動かす}」という記事を書きました。今回はその続きとして、もう少し実践的な内容をお届けします。前回の記事と併せてお読みください。

本稿の内容はs110.coreserver.jp上のPHP 5.4.21でFuelPHP 1.7.1を使って動作確認をしてあります。以降、ユーザー名をhogefuga、サーバーをs1024.coreserver.jpと仮定して説明します。みなさんのアカウント情報に適宜読み換えながら試してみてください。

== ローカルの開発環境でデータベースを使う

XREA/CORESERVERではMySQLとPostgreSQLが利用可能ですが、セキュリティーの理由からか、同一サーバーからのアクセス以外は弾かれるようで、ローカルの開発環境からサーバー上のデータベースを読み書きすることはできません。そこで開発用のデータベースをローカルに立て、ローカル環境ではローカルデータベースを、サーバー上の運用環境ではサーバーのデータベースを参照するようにしましょう。

まず、サーバー上の環境をproduction（運用環境）に設定しましょう。FuelProject/public/.htaccessの先頭、「AddHandler application/x-httpd-php54cgi .php」の上あたりに、次の行を追加します。

//emlist{
SetEnv FUEL_ENV production
//}

.htaccessはローカルとサーバーでは記述が異なるので、不用意な上書きに注意してください。

これでローカル環境はdevelopment、サーバー環境はproductionと別々のFUEL_ENVが設定されました。では、それぞれの環境用にdb.phpを作ります。

私はPostgreSQLが好きなので、基本的にサーバーにもPostgreSQLでデータベースを作成しています。ここではPostgreSQLの例を挙げますが、MySQLでも同様に設定できますので、参考にしてください。

まず、fuel/app/config/db.phpに共通設定を書きます。

//emlist{
<?php

return [
    'default' => [
        'type' => 'pdo',
        'identifier' => '',
        'table_prefix' => '',
        'charset' => 'utf8',
        'enable_cache' => true,
        'profiling' => false,
    ],
];
//}

続いてサーバー側の設定をfuel/app/config/production/db.phpに記述しましょう。ホスト名はs1024.coreserver.jpとせず、localhostにします。

//emlist{
<?php

return [
    'default' => [
        'connection' => [
            'dsn'        => 'pgsql:host=localhost;dbname=hogefuga',
            'username'   => 'hogefuga',
            'password'   => 'your_password',
            'persistent' => false,
            'compress'   => false,
        ],
    ],
];
//}

同様に、ローカル環境の設定をfuel/app/config/development/db.phpに記述します。

//emlist{
<?php

return [
    'default' => [
        'connection' => [
            'dsn'        => 'pgsql:host=localhost;dbname=hogefuga_local',
            'username'   => 'hogefuga_local',
            'password'   => 'your_local_password',
            'persistent' => false,
            'compress'   => false,
        ],
    ],
];
//}

ローカルに立てたデータベースのDB名とユーザー名、パスワードをサーバーのものと同じにすれば設定を分けずに済みますが、ローカル環境で複数のデータベースを切り替えて開発できるようにしておくほうがよいでしょう。

== タスクとEmailパッケージを利用してログファイルの肥大化を防ぐ

fuel/app/logsに日々蓄積されるログファイルを、FuelPHPを使って定期的に掃除させましょう。

次のような仕様を考えます。

 * 日付が変わったら、前日のログファイルをメールで送信する。
 * 1週間前のログファイルは削除する。その際、カラになったディレクトリも削除する。

cronジョブによるバッチ処理を毎晩走らせましょう。FuelPHPではタスクという機能でバッチ処理が実現できます。

適当なプロジェクトを作成し、fuel/app/tasks/logsender.phpというファイルを作成します。プロジェクトの設置場所は@<tt>{~/php/logsender}とします。

//emlist{
<?php
// @TODO ライセンス明記

/**
 * FuelPHP の前日分ログファイルをメールで送信し、1 週間前のログファイルを削除するタスク。
 */

namespace Fuel\Tasks;

\Package::load('email');

/**
 * ログのメール送信と古いログの削除を行うクラス。
 */
class LogSender
{
    /**
     * バッチ本体
     */
    public static function run()
    {
        // メールヘッダー情報
        $address_from = 'hogefuga@s1024.coreserve.jp';
        $name_from = 'FuelPHP Task';
        $address_to = 'hogefuga@s1024.coreserver.jp';
        $name_to = '俺';

        $yesterday = date('Y/m/d', time() - 86400);
        $one_week_ago = date('Y/m/d', time() - 86400 * 7);

        $log_dir = '/virtual/hogefuga/php/logsender/fuel/app/logs/';
        $yesterday_log_name = $log_dir . $yesterday . '.php';

        // メール送信
        $subject = "FuelPHP Log - $yesterday.php";

        $email = \Email::forge();
        $email->from($address_from, $name_from);
        $email->to($address_to, $name_to);
        $email->subject($subject);
        if (file_exists($yesterday_log_name)) {
            $email->body('');
            $email->attach($yesterday_log_name);
        } else {
            $email->body('昨日のログファイルはありません。');
        }
        try {
            $email->send();
        } catch (Exception $ex) {

        }

        // 1 週間前のログを削除
        $log_to_delete = $log_dir . $one_week_ago . '.php';
        if (file_exists($log_to_delete)) {
            if (!unlink($log_to_delete)) {
                echo 'ファイル削除に失敗' . PHP_EOL;
            }
            self::remove_directory(dirname($log_to_delete));
            self::remove_directory(dirname(dirname($log_to_delete)));
        }
    }


    /**
    * ディレクトリを削除する。ディレクトリ内にファイルがある時は何もしない。
    * @param string $dir  ディレクトリ名
    */
   private static function remove_directory($dir)
   {
       $files = scandir($dir);
       if (count($files) === 2) {
           rmdir($dir);
       }
    }
}
//}

このタスクをコマンドラインから実行する際、以下のコマンドではエラーが返ってきます。

//cmd{
php ~/php/logsender/oil r logsender
//}

本稿執筆時点ではXREA（一部のサーバーを除く）とCORESERVERはPHPのバージョンが5.2なので、FuelPHPは動かないのでした。PHPをフルパスで指定しましょう。

ここでサーバーにログインし@<fn>{login}、ホームディレクトリで@<tt>{ls /usr/local/bin/php*}を実行してみてください。以下のような表示になると思います。

//footnote[login][「@<href>{http://d.hatena.ne.jp/suno88/20131006/1381040481,レンタルサーバーXREA/CORESERVERでFuelPHPを動かす}」の「3. シンボリックリンクの作成」を参照してください。]

//cmd{
hogefuga@s1024:~> ls /usr/local/bin/php*
/usr/local/bin/php           /usr/local/bin/php-5.2.4cli   /usr/local/bin/php-5.4.19cli  /usr/local/bin/php55-config
/usr/local/bin/php4          /usr/local/bin/php-5.2.5      /usr/local/bin/php-5.4.21     /usr/local/bin/php55ize
/usr/local/bin/php-4.4.7     /usr/local/bin/php-5.2.5cli   /usr/local/bin/php-5.4.21cli  /usr/local/bin/php5cli
/usr/local/bin/php-4.4.7cli  /usr/local/bin/php52cli       /usr/local/bin/php-5.4.5      /usr/local/bin/php5-config
/usr/local/bin/php-4.4.8     /usr/local/bin/php53          /usr/local/bin/php-5.4.5cli   /usr/local/bin/php5ize
/usr/local/bin/php-4.4.8cli  /usr/local/bin/php-5.3.15     /usr/local/bin/php-5.4.7      /usr/local/bin/php6
/usr/local/bin/php4cli       /usr/local/bin/php-5.3.15cli  /usr/local/bin/php-5.4.7cli   /usr/local/bin/php6cli
/usr/local/bin/php4-config   /usr/local/bin/php-5.3.17     /usr/local/bin/php54cli       /usr/local/bin/php6-config
/usr/local/bin/php4ize       /usr/local/bin/php-5.3.17cli  /usr/local/bin/php54-config   /usr/local/bin/php6ize
/usr/local/bin/php5          /usr/local/bin/php-5.3.27     /usr/local/bin/php54ize       /usr/local/bin/php_back
/usr/local/bin/php52         /usr/local/bin/php-5.3.27cli  /usr/local/bin/php55          /usr/local/bin/php-cgi
/usr/local/bin/php-5.2.2     /usr/local/bin/php53cli       /usr/local/bin/php-5.5.3      /usr/local/bin/php-config
/usr/local/bin/php-5.2.2cli  /usr/local/bin/php53-config   /usr/local/bin/php-5.5.3cli   /usr/local/bin/phpize
/usr/local/bin/php-5.2.3     /usr/local/bin/php53ize       /usr/local/bin/php-5.5.5
/usr/local/bin/php-5.2.3cli  /usr/local/bin/php54          /usr/local/bin/php-5.5.5cli
/usr/local/bin/php-5.2.4     /usr/local/bin/php-5.4.19     /usr/local/bin/php55cli
hogefuga@s1024:~>
//}

FuelPHPの実行にはPHP 5.3以上であればいいので、好きなバージョンのPHPを指定しましょう。

//cmd{
/usr/local/bin/php-5.5.5cli ~/php/logsender/oil r logsender
//}

これを実行するシェルスクリプトを適当な場所に設置します。ここでは~/cron_logsender.shとして置くことにします。改行コードをLFにするのを忘れないようにしてください。

//emlist{
#!bin/sh
/usr/local/bin/php-5.5.5cli ~/php/logsender/oil r logsender
exit
//}

これを cron ジョブで起動するように設定します。CORESERVERのコントロールパネルにログインして、左メニューの「CRONジョブ」をクリックし、「CRONジョブの編集」画面を開きます。

//image[coreserver][CORESERVER コントロールパネル メニュー]{
//}

//image[coreserver2][CORESERVER コントロールパネル CRONジョブ設定]{
//}

空いている設定欄に、以下のように入力します。

 * 分 → 30
 * 時 → 0
 * 日 → *
 * 月 → *
 * 曜日 → *

その下にある「/virtual/hogefuga/」に続くテキストボックスには、

//emlist{
cron_logsender.sh > /dev/null 2>&1
//}

と入力します。これで、毎日0:30に先ほどのタスクが実行されます。

メール送信時にエラーが発生したときの処理などは、みなさんで追加してみてください。

=== 【12/18追記】

添付ファイルのサイズの制限について、@<href>{https://twitter.com/ounziw/status/413225966786199552,Fumito Mizunoさんから質問をいただきました}。公式発表はないようなので、実際にどのくらいまで添付で送れるか、簡単なコードを書いて調べてみました。

1MBから1MB刻みで添付ファイルのサイズを増やして送信したところ、こんなエラーメッセージが出ました。

//quote{
Fatal error: Allowed memory size of 94371840 bytes exhausted (tried to allocate 28329953 bytes) in /virtual/hogefuga/php/logsender/fuel/packages/email/classes/email/driver.php on line 965  

Fatal Error - Allowed memory size of 94371840 bytes exhausted (tried to allocate 28329953 bytes) in PKGPATH/email/classes/email/driver.php on line 965
//}

19MB の添付ファイルがついたメールまでは届きましたが、20MBの添付ファイルつきのメールは届きませんでした。サーバーによって差はあると思いますが、ログが20MB以上になると厳しいかもしれません。

本当はログファイルを ZIP 圧縮してから添付したいのですが、XREA/CORESERVERではZipArchiveクラスが使えないので、非圧縮のまま添付しています。

== 終わりに

この記事を最初に書いたとき、メール送信部分は素のPHPで、ログの内容をメール本文としてmb_send_mail()で送信するコードでした。書き終えた後、「FuelPHP のアドベントカレンダーなのにFuelPHP成分が少ないぞ」と思い直し、使ったこともないEmailパッケージで送信するコードを書き始めたところ、ものの5分程度で添付ファイルつきのメールを送信するコードが動いてしまい、ますますFuelPHPが気に入ってしまいました。

//quote{
@<strong>{@suno88}

@TODO

Twitter: @<href>{https://twitter.com/suno88,@suno88}

Blog: @<href>{http://d.hatena.ne.jp/suno88/,http://d.hatena.ne.jp/suno88/}
//}
