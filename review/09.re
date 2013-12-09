
= AspectMockでFuelPHPのアプリを100％テスト可能にする


@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の9日目です。


今日は、いま話題のAspectMockをFuelPHPで使い、FuelPHPのアプリを100％テストできないかというお話です。


ちなみに、本当に100%テストできるかどうかはまだ定かではありません。あと、カバー率を100%にすること自体は目的ではないので、あまり気にする必要はないと思います。

//quote{
"Testability" should not be used as argument deciding what design pattern is right to use and what is not.
//}


訳：どのデザインパターンが適切か否かという議論に、「テスト可能性」が使われるべきではない。


これが、AspectMockからの主張です。


なお、この記事の前提環境は、以下のとおりです。

 * FuelPHP 1.7.1
 * AspectMock masterブランチ cc2be6945a705e65a2a4a12df7e35de82d0129f7 (2013-09-09)


== 準備


AspectMockと必要なライブラリをComposerで追加します。

//emlist{
diff --git a/composer.json b/composer.json
index e1b21ea..006ac5b 100644
--- a/composer.json
+++ b/composer.json
@@ -16,10 +16,14 @@
         "forum": "http://fuelphp.com/forums"
     },
     "require": {
-        "php": ">=5.3.3",
+        "php": ">=5.4",
         "monolog/monolog": "1.5.*",
        "fuelphp/upload": "2.0.1"
     },
+    "require-dev": {
+        "codeception/aspect-mock": "*",
+        "symfony/finder":"*"
+    },
     "suggest": {
         "mustache/mustache": "Allow Mustache templating with the Parser package",
         "smarty/smarty": "Allow Smarty templating with the Parser package",
//}


インストールします。

//emlist{
$ php composer.phar update
//}

=== FuelPHP 1.7.1の変更


テスト実行の場合、AspectMockを使うようにFuelPHPを変更します。

//emlist{
diff --git a/fuel/app/bootstrap.php b/fuel/app/bootstrap.php
index a6213d5..b491688 100644
--- a/fuel/app/bootstrap.php
+++ b/fuel/app/bootstrap.php
@@ -1,9 +1,5 @@
 <?php

-// Load in the Autoloader
-require COREPATH.'classes'.DIRECTORY_SEPARATOR.'autoloader.php';
-class_alias('Fuel\\Core\\Autoloader', 'Autoloader');
-
 // Bootstrap the framework DO NOT edit this
 require COREPATH.'bootstrap.php';

diff --git a/oil b/oil
index 62033d6..4a21f80 100644
--- a/oil
+++ b/oil
@@ -48,6 +48,10 @@ define('COREPATH', realpath(__DIR__.'/fuel/core/').DIRECTORY_SEPARATOR);
 defined('FUEL_START_TIME') or define('FUEL_START_TIME', microtime(true));
 defined('FUEL_START_MEM') or define('FUEL_START_MEM', memory_get_usage());

+// Load in the Autoloader
+require COREPATH.'classes'.DIRECTORY_SEPARATOR.'autoloader.php';
+class_alias('Fuel\\Core\\Autoloader', 'Autoloader');
+
 // Boot the app
 require APPPATH.'bootstrap.php';

diff --git a/public/index.php b/public/index.php
index e01d3a4..9cb90d3 100644
--- a/public/index.php
+++ b/public/index.php
@@ -40,6 +40,10 @@ define('COREPATH', realpath(__DIR__.'/../fuel/core/').DIRECTORY_SEPARATOR);
 defined('FUEL_START_TIME') or define('FUEL_START_TIME', microtime(true));
 defined('FUEL_START_MEM') or define('FUEL_START_MEM', memory_get_usage());

+// Load in the Autoloader
+require COREPATH.'classes'.DIRECTORY_SEPARATOR.'autoloader.php';
+class_alias('Fuel\\Core\\Autoloader', 'Autoloader');
+
 // Boot the app
 require APPPATH.'bootstrap.php';
//}


AspectMockがロードされるようにし、AscpectMockの設定をします（$kernel->init()の部分）。ここが正しくないとAspectMockが正常に動作しません。

//emlist{
diff --git a/bootstrap_phpunit.php b/bootstrap_phpunit.php
index 3b5b851..2605850 100644
--- a/bootstrap_phpunit.php
+++ b/bootstrap_phpunit.php
@@ -32,6 +32,34 @@ unset($app_path, $core_path, $package_path, $_SERVER['app_path'], $_SERVER['core
 defined('FUEL_START_TIME') or define('FUEL_START_TIME', microtime(true));
 defined('FUEL_START_MEM') or define('FUEL_START_MEM', memory_get_usage());

+/**
+ * Load the Composer autoloader if present
+ */
+defined('VENDORPATH') or define('VENDORPATH', realpath(COREPATH.'..'.DS.'vendor').DS);
+if ( ! is_file(VENDORPATH.'autoload.php'))
+{
+   die('Composer is not installed. Please run "php composer.phar update" in the root to install Composer');
+}
+require VENDORPATH.'autoload.php';
+
+// Add AspectMock
+$kernel = \AspectMock\Kernel::getInstance();
+$kernel->init([
+   'debug' => true,
+   'appDir'    => __DIR__ . '/../',
+   'includePaths' => [
+       __DIR__.'/../app', __DIR__.'/../core', __DIR__.'/../packages',
+   ],
+   'excludePaths' => [
+   __DIR__.'/../app/tests', __DIR__.'/../core/tests',
+   ],
+]);
+
+// Load in the Autoloader
+//require COREPATH.'classes'.DIRECTORY_SEPARATOR.'autoloader.php';
+$kernel->loadFile(COREPATH.'classes'.DIRECTORY_SEPARATOR.'autoloader.php'); // path to your autoloader
+class_alias('Fuel\\Core\\Autoloader', 'Autoloader');
+
 // Boot the app
 require_once APPPATH.'bootstrap.php';
//}


AspectMockが動作するように、fuel/core/phpunit.xmlをfuel/app/phpunit.xmlにコピーし、backupGlobalsをfalseに変更します。

//emlist{
--- fuel/core/phpunit.xml   2013-12-02 19:56:52.847375706 +0900
+++ fuel/app/phpunit.xml    2013-12-02 19:56:38.910935143 +0900
@@ -1,6 +1,6 @@
 <?xml version="1.0" encoding="UTF-8"?>

-<phpunit colors="true" stopOnFailure="false" bootstrap="../core/bootstrap_phpunit.php">
+<phpunit colors="true" stopOnFailure="false" bootstrap="../core/bootstrap_phpunit.php" backupGlobals="false">
    <php>
        <server name="doc_root" value="../../"/>
        <server name="app_path" value="fuel/app"/>
//}

=== FuelPHP 1.8/develop（1.7.2以降）の変更


1.8/developブランチでは、上記の変更が取り込まれていますので、fuel/coreのbootstrap_phpunit.phpを変更し、

//emlist{
diff --git a/bootstrap_phpunit.php b/bootstrap_phpunit.php
index 50b9c88..20678e7 100644
--- a/bootstrap_phpunit.php
+++ b/bootstrap_phpunit.php
@@ -40,8 +40,22 @@ if ( ! is_file(VENDORPATH.'autoload.php'))
 }
 require VENDORPATH.'autoload.php';

+// Add AspectMock
+$kernel = \AspectMock\Kernel::getInstance();
+$kernel->init([
+       'debug' => true,
+       'appDir' => __DIR__ . '/../',
+       'includePaths' => [
+               __DIR__.'/../app', __DIR__.'/../core', __DIR__.'/../packages',
+       ],
+       'excludePaths' => [
+               __DIR__.'/../app/tests', __DIR__.'/../core/tests',
+       ],
+]);
+
 // Load in the Fuel autoloader
-require COREPATH.'classes'.DIRECTORY_SEPARATOR.'autoloader.php';
+//require COREPATH.'classes'.DIRECTORY_SEPARATOR.'autoloader.php';
+$kernel->loadFile(COREPATH.'classes'.DIRECTORY_SEPARATOR.'autoloader.php'); // path to your autoloader
 class_alias('Fuel\\Core\\Autoloader', 'Autoloader');

 // Boot the app
//}


fuel/app/phpunit.xmlを作成するだけでokです。

//emlist{
--- fuel/core/phpunit.xml   2013-12-02 19:56:52.847375706 +0900
+++ fuel/app/phpunit.xml    2013-12-02 19:56:38.910935143 +0900
@@ -1,6 +1,6 @@
 <?xml version="1.0" encoding="UTF-8"?>

-<phpunit colors="true" stopOnFailure="false" bootstrap="../core/bootstrap_phpunit.php">
+<phpunit colors="true" stopOnFailure="false" bootstrap="../core/bootstrap_phpunit.php" backupGlobals="false">
    <php>
        <server name="doc_root" value="../../"/>
        <server name="app_path" value="fuel/app"/>
//}

== テストの書き方


コントローラからResponse::redirect()でリダイレクトする場合のテストを作成してみましょう。


Response::redirect()は安全のため内部でexit()しているため、そのままではテストがそこで終了してしまい、テストできません。これをテストダブルで置き換えて、テスト可能にします。


まず、以下のようなコントローラを作成します。

//emlist{
<?php

class Controller_Test extends Controller
{
    public function action_redirect()
    {
        return Response::redirect('welcome/404', 'location', 404);
    }
}
//}


続いてテストを作成しましょう。

//emlist{
<?php

// AspectMockのTestクラスをtestとしてインポート
use AspectMock\Test as test;

/**
 * Tests for Controller_Test
 *
 * @group App
 * @group Controller
 */
class Test_Controller_Test extends TestCase
{
    protected function tearDown()
    {
        test::clean(); // 登録したテストダブルをすべて削除
    }

    public function test_redirect()
    {
        // Response::redirect()を単にtrueを返すテストダブルに置き換え
        $req = test::double('Fuel\Core\Response', ['redirect' => true]);

        // 'test/redirect'へのリクエストを生成
        $response = Request::forge('test/redirect')
                        ->set_method('GET')->execute()->response();

        // Response::redirect()が以下の引数で実行されたことを確認
        $req->verifyInvoked('redirect', ['welcome/404', 'location', 404]);
    }
}
//}


テストを実行します。

//emlist{
$ oil test --group=App
Tests Running...This may take a few moments.
PHPUnit 3.7.28 by Sebastian Bergmann.

Configuration read from /mnt/fuelphp/fuel/app/phpunit.xml

.

Time: 8.08 seconds, Memory: 48.50Mb

OK (1 test, 0 assertions)
//}


通りました。「0 assertinos」というのがちょっと変ですが、PHPUnitの検証メソッドを使っていないため、いたしかたありません。


試しに、verifyInvoked()での第3引数の指定を404から405に変更してみます。

//emlist{
$req->verifyInvoked('redirect', ['welcome/404', 'location', 405]);

$ oil test --group=App
Tests Running...This may take a few moments.
PHPUnit 3.7.28 by Sebastian Bergmann.

Configuration read from /mnt/fuelphp/fuel/app/phpunit.xml

.

Time: 8.08 seconds, Memory: 48.50Mb

OK (1 test, 0 assertions)
[vagrant@localhost fuelphp]$ oil test --group=App
Tests Running...This may take a few moments.
PHPUnit 3.7.28 by Sebastian Bergmann.

Configuration read from /mnt/fuelphp/fuel/app/phpunit.xml

F

Time: 7.72 seconds, Memory: 48.50Mb

There was 1 failure:

1) Test_Controller_Test::test_redirect
Expected Fuel\Core\Response::redirect('welcome/404','location',405) to be invoked but it never occur.

/mnt/fuelphp/fuel/vendor/codeception/aspect-mock/src/AspectMock/Proxy/Verifier.php:73
/mnt/fuelphp/fuel/app/tests/controller/test.php:28

FAILURES!
Tests: 1, Assertions: 0, Failures: 1.
//}


正しく失敗しました。


このように、AspectMockを使うと静的メソッドをテストダブルに置き換えたり、メソッドを動的に再定義して、簡単にテストすることができます。AspectMockの主張どおり、テスト可能にするためだけにDIを使う必要はなくなります。


ただし、AspectMockは「Stability: alpha」となっており、まだ発展途上のツールのようですので期待したとおりに動作しない可能性はあります。

== 参考

 * @<href>{https://github.com/Codeception/AspectMock,https://github.com/Codeception/AspectMock}
 * @<href>{http://codeception.com/07-31-2013/nothing-is-untestable-aspect-mock.html,Nothing is Untestable: AspectMock in Action}
 * @<href>{http://codeception.com/09-13-2013/understanding-aspectmock.html,Understanding AspectMock}


== おまけ


このFuelPHP Advent Calendarは今年で3年目ですが、一昨年、去年の分は、電子書籍化されてIT系の有名出版社より出版されています。

 * @<href>{https://gihyo.jp/dp/sp/advent2011/G11C13,『FuelPHP Advent Calendar 2011』【電子書籍】} 技術評論社
 * @<href>{http://tatsu-zine.com/books/fuelphpadvent2012,『FuelPHP Advent Calendar 2012』【電子書籍】} 達人出版会



いずれも無料でダウンロードできますので、まだ、読んでいないFuelPHPユーザの方は、読んでみるといろいろな発見があると思いますよ。
