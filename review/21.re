
= FuelPHPをRocketeerで自動デプロイしてみる。マイグレーションとPHPUnitも実行してみる。

2014/1/25 追記: 先日、Rocketeerのバージョンが1.0.0になりました。当記事の内容は、それよりも古いバージョンで確認しています。当記事記載のサンプルレポジトリは1.0.0に対応済みです。詳しはそちらのコミットログを御覧ください。

@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013} 21日目です。@<href>{https://twitter.com/madmamor,@madmamor}が担当します。@<br>{}


今日は、PHP製デプロイツール「Rocketeer」を使って、FuelPHPをコマンド一つでデプロイしてみます。デプロイする最中に、PHPUnitやマイグレーションも実行してみます。


 * Rocketeer公式ドキュメント：@<href>{http://rocketeer.autopergamene.eu/,http://rocketeer.autopergamene.eu/}

ライセンスファイルへのリンクが切れてしまっていますが、MITライセンスと書かれています。

 * RocketeerのGitHub：@<href>{https://github.com/Anahkiasen/rocketeer,https://github.com/Anahkiasen/rocketeer}

 今回の内容は、MacOS X Mavericks（以下、ローカル）とVagrantで起動しているUbuntu 13.10（以下、リモート）な環境で確認しています。ローカルは普段通りな開発を行う場所で、そこからコマンドを実行して、リモートにデプロイするイメージです。FuelPHPは1.7.1を使いましたが、Composer対応以降のバージョンであれば、あまり関係は無いはずです。


記事内のソースのライセンスについては、Rocketeerが生成するファイルはRocketeerのライセンスに準じます。私が作成したファイルは、ソースにも書きますが、WTFPLライセンスにします。

 * @<href>{http://www.wtfpl.net/txt/copying/,http://www.wtfpl.net/txt/copying/}

== 1. 下準備(ローカル)


php.iniで以下の設定をします。

#@# lang: .brush:text
//emlist{
phar.readonly = Off
//}


これをしないと、後述のrocketeer.pharが自身の内部を更新する関係か、以下の警告が大量に出ました。

#@# lang: .brush:text
//emlist{
failed to open stream: phar error: write operations disabled by the php.ini setting phar.readonly
//}


併せて、FuelPHPプロジェクトを作成して、Gitリポジトリへコミットしておいて下さい。このGitリポジトリはリモート側からアクセスできる必要があります。アクセスには、ユーザ名とパスワード、あるいはユーザ名と鍵ファイル(と鍵のパスワード)による認証が使えます。@<br>{}

注意：リモートで鍵の設定時~/.ssh/configに以下が無いとエラーになる可能性が有ります。あるいは、一度手動でcloneして、ホストの登録を済ませておきましょう。

#@# lang: .brush:text
//emlist{
StrictHostKeyChecking no
//}


尚、この記事を作成するにあたって作成したFuelPHPプロジェクトのサンプルを公開してあります。

 * @<href>{https://github.com/mp-php/fuelphp-advent-calendar-2013-rocketeer-sample,https://github.com/mp-php/fuelphp-advent-calendar-2013-rocketeer-sample}

== 2. 下準備(リモート)


Git、PHPとmcrypt extension、Composerをインストールしておきます。更に、以下のsymlinkを貼っておきます。このリンク先は、今現在は存在しませんが、それで構いません。

#@# lang: .brush:text
//cmd{
$ sudo ln -s /home/vagrant/www/fuel-rocketeer-sample/current/public /var/www/fuel-rocketeer-sample
//}

== 3. Rocketeerのインストールと設定ファイルの準備


ローカルで、以下をダウンロードして、プロジェクトルートに配置します。ダウンロード方法は何でも構いません。


 * http://rocketeer.autopergamene.eu/versions/rocketeer.phar


以下のコマンドでコマンド一覧やヘルプが表示できればRocketeerのインストールは完了です。

#@# lang: .brush:text
//cmd{
$ php rocketeer.phar    # コマンド一覧
$ php rocketeer.phar -h # ヘルプ
//}


次に、設定ファイルを準備します。以下のコマンドを実行して下さい。設問は、とりあえず全て未入力でEnterで良いです。

#@# lang: .brush:text
//cmd{
$ php rocketeer.phar ignite
//}


以下のファイルが生成されたはずです。

 * rocketeer/config.php ... 主にリモートの接続情報を設定する
 * rocketeer/hooks.php ... 主にデプロイ時等のbefore/afterのタスクを設定する（今回は使いません）
 * rocketeer/paths.php ... phpやcomposer等のコマンドのパスを設定する
 * rocketeer/remote.php ... リモートのデプロイ先に関する色々な設定をする
 * rocketeer/scm.php ... Gitリポジトリの設定をする（SVNも使えるみたいです）
 * rocketeer/stages.php ... 同一サーバに複数ステージ（stagingやproduction）がある場合に使う？（今回は使いません）



注意: rocketeer.pharは自身の内部にキャッシュ的に接続設定を保存するようです。以降の設定が正しく反映されない場合、以下のコマンドを実行してみてください。

#@# lang: .brush:text
//cmd{
$ php rocketeer.phar flush
//}


また、その性質上、rocketeer.pharをパブリックなリポジトリにコミットするのはリスクが有るかもしれません。.gitignoreで除外してしまうのも有りかと思います。

== 4. リモートの接続情報を設定して確認してみる


rocketeer/config.phpを修正します。以下は例なので、適切に書き換えて下さい（以降、同様です）。

#@# lang: .brush:text
//emlist{
'connections' => array(
    'production' => array(
        'host'      => '192.168.33.10',
        'username'  => 'vagrant',
        'password'  => '',
        'key'       => '/Users/mamor/.vagrant.d/insecure_private_key',
        'keyphrase' => '',
    ),
),
//}


SSHのポートを22以外にしている場合は"xxx.yyy.com:2222"のように指定してあげればOKです。


早速、正しく設定できたか確認してみましょう。

#@# lang: .brush:text
//cmd{
$ php rocketeer.phar check

No repository is set for the repository, please provide one :
No username is set for the repository, please provide one :
No password is set for the repository, please provide one :
Checking presence of git
Checking PHP version
Checking presence of Composer
Checking presence of mcrypt extension
Your server is ready to deploy
Execution time: 0.8238s
//}


正しく接続できて、gitコマンド、PHPバージョン、composerコマンド、mcrypt extensionのチェックが行われました。必要なPHPバージョンは、すみません、確認していません。が、Rocketeerのcomposer.jsonには"php": ">=5.3.0"と書かれています。ちなみに手元は5.5です。

== 5. デプロイの設定をしてデプロイしてみる


rocketeer/remote.phpを修正します。

#@# lang: .brush:text
//emlist{
diff --git a/rocketeer/remote.php b/rocketeer/remote.php
index a21279b..51424c4 100644
--- a/rocketeer/remote.php
+++ b/rocketeer/remote.php
@@ -11,12 +11,12 @@
        ),

        // The root directory where your applications will be deployed
-       'root_directory'   => '/home/www/',
+       'root_directory'   => '/home/vagrant/www/',

        // The name of the application to deploy
        // This will create a folder of the same name in the root directory
        // configured above, so be careful about the characters used
-       'application_name' => '',
+       'application_name' => 'fuel-rocketeer-sample',

        // The number of releases to keep at all times
        'keep_releases'    => 4,
@@ -25,23 +25,24 @@
        // Use this to list folders that need to keep their state, like
        // user uploaded data, file-based databases, etc.
        'shared' => array(
-               '{path.storage}/logs',
-               '{path.storage}/sessions',
+               'fuel/app/cache',
+               'fuel/app/logs',
+               'fuel/app/tmp',
        ),

        'permissions' => array(

                // The permissions to CHMOD folders to
                // Change to null to leave the folders untouched
-               'permissions' => 755,
+               'permissions' => 777,

                // The folders and files to set as web writable
                // You can pass paths in brackets, so {path.public} will return
                // the correct path to the public folder
                'files' => array(
-                       'app/database/production.sqlite',
-                       '{path.storage}',
-                       '{path.public}',
+                       'fuel/app/cache',
+                       'fuel/app/logs',
+                       'fuel/app/tmp',
                ),

                // The web server user and group to CHOWN folders to
//}


"root_directory"の下に"application_name"な名前のディレクトリが作成され、その中にデプロイされます。この例だと"/home/vagrant/www/fuel-rocketeer-sample"になりますね。


"shared"では、デプロイをまたいで共有したいディレクトリやファイルを設定します。大抵の場合、ログディレクトリやキャッシュディレクトリ等、.gitignoreに書かれているものになると思います。裏を返せば、例えばデプロイ毎にキャッシュをクリアしたければ、あえて共有しなければOKです。尚、共有はsymlinkによって実現されます。@<br>{}


注意：ディレクトリを共有する場合、ディレクトリそのものがリポジトリに含まれている必要があります。.gitkeepや、以下のような.gitignoreファイルをそのディレクトリに入れるなどしておいて下さい。

#@# lang: .brush:text
//emlist{
*
!.gitignore
//}


"permissions"は、指定したディレクトリやファイルを、指定したパーミッションに変更します。今回の例では777を指定していますが、適切な値を設定するようにお願いします。


次にrocketeer/scm.phpを修正します。

#@# lang: .brush:text
//emlist{
'repository' => 'https://github.com/mp-php/fuelphp-advent-calendar-2013-rocketeer-sample.git',
//}


"repository"に、GitのリポジトリURLを設定します。今回の例はGitHub上のリポジトリなので、usernameとpasswordは空のままで構いません。また、ファイル内のコメントにも書かれているように、既に鍵認証の設定がされている場合も、空で構いません。


以上で基本的な設定が済んだので、お待ちかねのデプロイを実行してみましょう。

#@# lang: .brush:text
//cmd{
$ php rocketeer.phar deploy

No username is set for the repository, please provide one :
No password is set for the repository, please provide one :
Cloning repository in "/home/vagrant/www/fuel-rocketeer-sample/releases/20131220204831"
Initializing submodules if any
Installing Composer dependencies
Setting permissions for /home/vagrant/www/fuel-rocketeer-sample/releases/20131220204831/fuel/app/cache
Setting permissions for /home/vagrant/www/fuel-rocketeer-sample/releases/20131220204831/fuel/app/logs
Setting permissions for /home/vagrant/www/fuel-rocketeer-sample/releases/20131220204831/fuel/app/tmp
Sharing file /home/vagrant/www/fuel-rocketeer-sample/releases/20131220204831/fuel/app/cache
Sharing file /home/vagrant/www/fuel-rocketeer-sample/releases/20131220204831/fuel/app/logs
Sharing file /home/vagrant/www/fuel-rocketeer-sample/releases/20131220204831/fuel/app/tmp
Successfully deployed release 20131220204831
No releases to prune from the server
Execution time: 61.1617s
//}


Gitリポジトリのclone（submodulesがあればそれも）が行われ、composer installが行われ、パーミッション変更が行われ、共有が行われました。


ブラウザからhttp://[ドメイン]/fuel-rocketeer-sample/にアクセスして、おなじみのトップ画面が表示されればデプロイ成功です。


ざっとディレクトリ構造を見てみましょう。

#@# lang: .brush:text
//cmd{
$ ll /home/vagrant/www/fuel-rocketeer-sample/
total 20
drwxrwxr-x 4 vagrant vagrant 4096 Dec 20 11:49 ./
drwxrwxr-x 3 vagrant vagrant 4096 Dec 20 11:48 ../
lrwxrwxrwx 1 vagrant vagrant   63 Dec 20 11:49 current -> /home/vagrant/www/fuel-rocketeer-sample/releases/20131220204831/
drwxrwxr-x 3 vagrant vagrant 4096 Dec 20 11:48 releases/
drwxrwxr-x 3 vagrant vagrant 4096 Dec 20 11:49 shared/
//}


"current"ディレクトリが、先ほどデプロイしたディレクトリへsymlinkされています。

#@# lang: .brush:text
//cmd{
$ ll /home/vagrant/www/fuel-rocketeer-sample/current/fuel/app/
total 56
drwxrwxr-x 11 vagrant vagrant 4096 Dec 20 11:49 ./
drwxrwxr-x  6 vagrant vagrant 4096 Dec 20 11:49 ../
-rw-rw-r--  1 vagrant vagrant  718 Dec 20 11:48 bootstrap.php
lrwxrwxrwx  1 vagrant vagrant   61 Dec 20 11:49 cache -> /home/vagrant/www/fuel-rocketeer-sample/shared/fuel/app/cache/
drwxrwxr-x  5 vagrant vagrant 4096 Dec 20 11:48 classes/
drwxrwxr-x  6 vagrant vagrant 4096 Dec 20 11:48 config/
drwxrwxr-x  3 vagrant vagrant 4096 Dec 20 11:48 lang/
lrwxrwxrwx  1 vagrant vagrant   60 Dec 20 11:49 logs -> /home/vagrant/www/fuel-rocketeer-sample/shared/fuel/app/logs/
drwxrwxr-x  2 vagrant vagrant 4096 Dec 20 11:48 migrations/
drwxrwxr-x  2 vagrant vagrant 4096 Dec 20 11:48 modules/
drwxrwxr-x  2 vagrant vagrant 4096 Dec 20 11:48 tasks/
drwxrwxr-x  5 vagrant vagrant 4096 Dec 20 11:48 tests/
lrwxrwxrwx  1 vagrant vagrant   59 Dec 20 11:49 tmp -> /home/vagrant/www/fuel-rocketeer-sample/shared/fuel/app/tmp/
drwxrwxr-x  2 vagrant vagrant 4096 Dec 20 11:48 vendor/
drwxrwxr-x  3 vagrant vagrant 4096 Dec 20 11:48 views/
//}


共有設定したディレクトリが、"shared"ディレクトリ下にsymlinkされています。パーミッションも、設定したとおりに変更されています。


以上が、Rocketeerによる基本的なデプロイ方法です。

== 6. マイグレーションも実行してみる


だいぶ長くなってきましたが、続いてマイグレーションの実行です。簡単なマイグレーションファイルを作成してコミットしておきます。

#@# lang: .brush:text
//cmd{
$ php oil generate migration create_users name:text email:string password:string
//}


リモート側でDBやDBユーザの作成、それに対するFuelPHPのconfigのdb.phpの設定も済ませておいて下さい。


次に、2ファイルを新規作成します。


まず、rocketeer/tasks/Migrate.phpを新規作成します。今回はサンプルなので、名前空間はつけていません。尚、"Rocketeer\Traits\Task"を継承しますが、このクラスはabstract classであってトレイトではないようです。

#@# lang: .brush:php
//emlist{
<?php

/**
 * Migrate class
 *
 * @author    Mamoru Otsuka http://madroom-project.blogspot.jp/
 * @copyright 2013 Mamoru Otsuka
 * @license   WTFPL License http://www.wtfpl.net/txt/copying/
 */
class Migrate extends Rocketeer\Traits\Task
{
    /**
     * @inheritDoc
     */
    protected $description = 'Migrates the database';

    /**
     * @inheritDoc
     */
    public function execute()
    {
        // 実行時にメッセージとして表示されます
        $this->command->info($this->description);

        // currentディレクトリ内でコマンドを実行します
        $output = $this->runForCurrentRelease('php oil r migrate');

        // 第一引数は失敗時のメッセージです
        // 第二引数は失敗時の詳細です
        // 第三引数は成功時のメッセージです
        return $this->checkStatus('Migrate failed', $output, 'Migrate successfully');
    }
}
//}


rocketeer/tasks.php を新規作成します。

#@# lang: .brush:php
//emlist{
<?php
/**
 * rocketeer/tasks.php
 *
 * @author    Mamoru Otsuka http://madroom-project.blogspot.jp/
 * @copyright 2013 Mamoru Otsuka
 * @license   WTFPL License http://www.wtfpl.net/txt/copying/
 */

// Migrateクラスをカスタムタスクとして登録します
// $ php rocketeer.phar migrate で個別に実行できます
require_once __DIR__.'/tasks/Migrate.php';
Rocketeer\Facades\Rocketeer::add('Migrate');

// deploy後に自動実行されます
// 自動実行したくない場合は書かないで下さい
Rocketeer\Facades\Rocketeer::after('deploy', 'Migrate');
//}


Gitリポジトリにコミット（Push）したら、デプロイを実行してみます。

#@# lang: .brush:text
//cmd{
$ php rocketeer.phar deploy
…略…
Migrates the database
Migrate successfully
Removing 1 release from the server
Execution time: 34.8705s
//}


マイグレーションも実行できました。以下のコマンドでマイグレーションのみを個別に実行することもできます。

#@# lang: .brush:text
//cmd{
$ php rocketeer.phar migrate
//}


注意：deployのafterタスクは、既にsymlinkが貼替えられていることに注意して下さい。尚、マイグレーションを行う場合は、別途、何らかの方法でメンテナンスモードに切り替えるタスクを作成する必要があるかと思います（もちろん、手作業でも良いですが）。また、後述のPHPUnit失敗時の挙動も気になるところで、マイグレーションを自動化するのはそれなりのリスクが伴いそうです。が、今回はとりあえず自動化して進めます。@<br>{}


after（やbefore）タスクにはクラスの他に、インラインによるコマンド設定や、クロージャの設定もできるみたいです（まだやったことはありません）。クラスにすると"$this->runForCurrentRelease('コマンド')"のように、便利なメソッドでパス周りの調整が簡単になるので、迷ったらクラスで良いのかなと思います。クラスだと、前述のように個別で実行もできますね。

== 7. PHPUnitも実行してみる


そろそろ最後です。ソースをcloneして、PHPUnitを実行して、全てのテストが成功したらデプロイ続行、1つでも失敗したらデプロイ中止（symlinkの貼替えを行わない）できたら良いですね。


composer.jsonに以下を追記します。

#@# lang: .brush:text
//emlist{
diff --git a/composer.json b/composer.json
index e1b21ea..5ef630e 100644
--- a/composer.json
+++ b/composer.json
@@ -20,6 +20,9 @@
         "monolog/monolog": "1.5.*",
        "fuelphp/upload": "2.0.1"
     },
+    "require-dev": {
+        "phpunit/phpunit": "3.*"
+    },
     "suggest": {
         "mustache/mustache": "Allow Mustache templating with the Parser package",
         "smarty/smarty": "Allow Smarty templating with the Parser package",
//}


プロジェクト直下にphpunit.xmlを用意します。fuel/core/phpunit.xmlをコピーして、パス周りを整えただけです。

#@# lang: .brush:xml
//emlist{
<?xml version="1.0" encoding="UTF-8"?>

<phpunit colors="true" stopOnFailure="false" bootstrap="fuel/core/bootstrap_phpunit.php">
    <php>
        <server name="doc_root" value="./"/>
        <server name="app_path" value="fuel/app"/>
        <server name="core_path" value="fuel/core"/>
        <server name="package_path" value="fuel/packages"/>
        <server name="vendor_path" value="fuel/vendor"/>
        <server name="FUEL_ENV" value="test"/>
    </php>
    <testsuites>
        <testsuite name="core">
            <directory suffix=".php">fuel/core/tests</directory>
        </testsuite>
        <testsuite name="packages">
            <directory suffix=".php">fuel/packages/*/tests</directory>
        </testsuite>
        <testsuite name="app">
            <directory suffix=".php">fuel/app/tests</directory>
        </testsuite>
    </testsuites>
</phpunit>
//}


rocketeer/paths.php を修正します。

#@# lang: .brush:text
//emlist{
diff --git a/rocketeer/paths.php b/rocketeer/paths.php
index f366b41..2b4d6d4 100644
--- a/rocketeer/paths.php
+++ b/rocketeer/paths.php
@@ -19,4 +19,6 @@
        // Path to the Artisan CLI
        'artisan'  => '',

+       // Path to PHPUnit
+       'phpunit' => 'fuel/vendor/bin/phpunit',
 );
//}


Rocketeerは/usr/local/bin等のグローバルな場所のphpunit、あるいはプロジェクト直下のvendor/bin/phpunitは勝手に見つけてくれます。FuelPHPの場合はfuel/vendor/bin/phpunitになるので、この設定が必要です（この設定がない場合は、対話式でパスの入力が可能ですが）。


-tオプションをつけてデプロイを実行してみます。

#@# lang: .brush:text
//cmd{
$ php rocketeer.phar deploy -t
…略…
Running tests...
Tests passed successfully
…略…
Execution time: 194.1824s
//}


テストが行われました。単体でも実行できます。

#@# lang: .brush:text
//cmd{
$ php rocketeer.phar test
…略…
Testing the application
Running tests...
…略…
[vagrant@192.168.33.11] (production) Time: 9.23 seconds, Memory: 23.25Mb
[vagrant@192.168.33.11] (production) OK (361 tests, 413 assertions)
Tests passed successfully
Execution time: 9.6588s
//}


注意：テストの実行をafterタスクで行うこともできますが、その時には既にsymlinkが貼替わってしまっています。-tオプションを使うようにしましょう。@<br>{}


最後に、必ず失敗するテストを作成して、どうなるかも確認してみます（ソースは割愛します）。

#@# lang: .brush:text
//cmd{
$ php rocketeer.phar deploy -t

No username is set for the repository, please provide one :
No password is set for the repository, please provide one :
Cloning repository in "/home/vagrant/www/fuel-rocketeer-sample/releases/20131220215555"
Initializing submodules if any
Installing Composer dependencies
Running tests...
Tests failed
PHPUnit 3.8-g5fb30aa by Sebastian Bergmann.

Configuration read from /home/vagrant/www/fuel-rocketeer-sample/releases/20131220215555/phpunit.xml

The Xdebug extension is not loaded. No code coverage will be generated.

...............................................................  63 / 362 ( 17%)
............................................................... 126 / 362 ( 34%)
............................................................... 189 / 362 ( 52%)
............................................................... 252 / 362 ( 69%)
............................................................... 315 / 362 ( 87%)
..............................................F

Time: 8.6 seconds, Memory: 23.25Mb

There was 1 failure:

1) Test_Example::test_fail

/home/vagrant/www/fuel-rocketeer-sample/releases/20131220215555/fuel/app/tests/example.php:14

FAILURES!
Tests: 362, Assertions: 413, Failures: 1.
Tests failed
Rolling back to release 20131220215158
Migrates the database
Migrate successfully
Execution time: 196.3031s
//}


デプロイが中断されました。symlinkは以前のままです。中断されたデプロイのディレクトリはゴミとして残りますが、rocketeer/remote.phpの"keep_releases"により、そのうち掃除されると思いますので、あまり気にしなくても良いかなと思います。マイグレーションが実行されてしまうのは想定外だったので、この点は今後の課題にします…

== 8. まとめ


（全ての機能を把握できているわけではありませんし、公式ドキュメントに記載されているプラグイン機能も気になるところですが）Rocketeerを使って、FuelPHPをコマンド1つで、PHPUnitやマイグレーションの実行を含めてデプロイできました。


デプロイツールはRuby製のCapistranoが有名ですが、RocketeerはPHP製ということもあり、ComposerやPHPUnitの扱いを標準でサポートしてくれていて助かります。今日現在、Rocketeerの使い方に関する日本語の情報はかなり少ない（というか無いかもしれません。あったらすみません）ので、今後、盛り上がってくれると良いなーと思います。


ChefとVagrantのおかげで、こういったサーバが絡む実験もやりやすくなったので、ぜひ試してみてください。@<br>{}


以上です。お疲れ様でした。

//quote{
@<strong>{@madmamor}

Developer & Bassist on TAMACENTER OUTSiDERS.

Twitter: @<href>{https://twitter.com/madmamor,@madmamor}

Blog: @<href>{http://madroom-project.blogspot.jp/,http://madroom-project.blogspot.jp/}
//}
