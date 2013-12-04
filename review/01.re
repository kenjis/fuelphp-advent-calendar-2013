
= FuelPHPをphar化してポータブルに！


@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の1日目の参加記事です。


初めましての方もご存知の方も、よろしくお願いします。


@<href>{https://twitter.com/sharkpp,@sharkpp}です。


さて、昨年の12月1日はアドベント（待降節）ではありませんでしたが、安心してください、今年は12月1日からアドベントは始まります。


とりあえず、初日なので軽い内容でいきたいと思います。


内容は、FuelPHPをphar（PHP Archive）で１ファイルにしてウェブサーバーで動かしてみよう、です。


子ネタをやりつつpharの紹介も兼ねています。


環境としては、

 * PHP 5.3 以上
 * FuelPHP 1.7
 * Apache on CentOS or Windows



を想定しています。

== phar（PHP Archive）ってご存知ですか？


まず、大前提。


PHPマニュアル（@<href>{http://php.net/manual/ja/intro.phar.php,http://php.net/manual/ja/intro.phar.php}）によると、

//quote{
phar拡張モジュールは、PHP アプリケーション全体をひとつの"phar"（PHP Archive）ファイルにまとめてしまい、配布やインストールを容易にするためのものです。
//}

//noindent
となっています。


実際に使われている例としては、

 * @<href>{http://getcomposer.org/,composer.phar}　パッケージ管理ツール
 * @<href>{https://github.com/fabpot/goutte,goutte.phar}　スクレイピングライブラリ
 * @<href>{https://github.com/guzzle/guzzle,guzzle.phar}　HTTPクライアントライブラリ
 * @<href>{http://pear2.php.net/PEAR2_Pyrus,pyrus.phar}　PEAR2

//noindent
などがあります。


例として上げた中でもcomposerはFuelPHPを使っている方であれば

//cmd{
$ php composer.phar update
//}

//noindent
と、このような形で触ったことがあると思います。

== FuelPHP をインストール


Pharクラスの中でも、今回は@<href>{http://jp2.php.net/manual/ja/phar.webphar.php,Phar::webPhar}を使います。


まずは、FuelPHPを適当なフォルダに配置します。
詳しい手順はFuelPHPドキュメント（@<href>{http://fuelphp.jp/docs/1.7/,http://fuelphp.jp/docs/1.7/}）に書かれているので参考にしてください。
ここでは、@<tt>{~/fuelphp-1.7}に配置されるものとします。

//cmd{
$ curl get.fuelphp.com/oil | sh
$ cd ~
$ oil create fuelphp-1.7
//}

//noindent
もしくは、

//cmd{
$ wget http://fuelphp.com/files/download/25 -O fuelphp.zip
$ unzip fuelphp.zip
//}

//noindent
とすることで、gitがインストールされていない場合はfuelphp.comからダウンロードして展開ができます。


次に

//cmd{
$ cd fuelphp-1.7
$ php composer.phar self-update
$ php composer.phar update
//}

//noindent
として、composer自身のアップデートとパッケージを更新します。


これで、Apacheなどのウェブサーバー上に公開するとWelcome画面が表示されるはずです。

== FuelPHPをPharで1ファイルにまとめる


まず、そのままでは1ファイルにまとめても動かないのでいくつかソースを変更する必要があります。


残念なことにcoreの中も変更する必要がありました。


インストール直後のページを表示できるようにするために変更するファイルは

 * @<tt>{public/index.php}
 * @<tt>{fuel/app/config/config.php}
 * @<tt>{fuel/app/config/asset.php} ※ fuel/core/config/asset.phpからコピー
 * @<tt>{fuel/core/bootstrap.php}
 * @<tt>{fuel/core/classes/file/area.php}

//noindent
の5個のファイルです。


実際のアプリケーションの場合は先に挙げたファイル以外にも変更が必要になると思います。


変更のポイントは、

 * phar内からのrealpathが常に空文字で返ってくるのでダミー関数に置き換え
 * Windows であっても パスの区切りは@<tt>{'/'}とする
 * パスに含まれる親ディレクトリへの移動などを削除し正規化
 * ログやキャッシュの保存先が.phar外を示すようにする

//noindent
と、主に、ファイルパスに関する物が主となります。


まず、@<tt>{public/index.php}の変更部分です。


パスを正規化する@<tt>{canonicalizePath}関数と@<tt>{realpath}関数のダミーとして@<tt>{realpat_}関数を定義しています。

//emlist{
 error_reporting(-1);
 ini_set('display_errors', 1);

 +function canonicalizePath($path) {
 +    $path = 0===strpos($path,'phar://')?'phar://'.preg_replace('!//!', '/', substr($path,7))
 +                                       :preg_replace('!//!', '/', $path);
 +    do {
 +        $tmp  = $path;
 +        $path = preg_replace('!/[^/]+/\.\./!', '/', $tmp);
 +    } while ($tmp != $path);
 +    return rtrim($path, '/');
 +}
 +
 +function realpat_($path) {
 +    return canonicalizePath(str_replace(array('/', '\\'), '/', $path));
 +}
//}


あとは、@<tt>{realpath}関数の代わりに@<tt>{realpat_}関数を使うようにし、パスの区切りも@<tt>{'/'}に変更しています。

//emlist{
-define('DOCROOT', __DIR__.DIRECTORY_SEPARATOR);
+define('DOCROOT', realpat_(__DIR__.'/'));
//}

//emlist{
-define('APPPATH', realpath(__DIR__.'/../fuel/app/').DIRECTORY_SEPARATOR);
+define('APPPATH', realpat_(__DIR__.'/../fuel/app/').'/');
//}

//emlist{
-define('PKGPATH', realpath(__DIR__.'/../fuel/packages/').DIRECTORY_SEPARATOR);
+define('PKGPATH', realpat_(__DIR__.'/../fuel/packages/').'/');
//}

//emlist{
-define('COREPATH', realpath(__DIR__.'/../fuel/core/').DIRECTORY_SEPARATOR);
+define('COREPATH', realpat_(__DIR__.'/../fuel/core/').'/');
//}


@<tt>{fuel/app/config/config.php}の変更部分です。.phar内には書き込めないので.pharと同じ場所の@<tt>{writable}ディレクトリを示すように変更しています。


@<strong>{保存先は公開ディレクトリ外を示すべきなので、さらに1つ上などに示すようにするのが本来は良いでしょう。}

//emlist{
-    // 'cache_dir'       => APPPATH.'cache/',
+    'cache_dir'       => canonicalizePath(str_replace('phar://', '', APPPATH).'../../../writable/cache/'),
//}

//emlist{
-    // 'log_path'         => APPPATH.'logs/',
+    'log_path'         => canonicalizePath(str_replace('phar://', '', APPPATH).'../../../writable/logs/'),
//}


@<tt>{fuel/app/config/asset.php}の変更部分です。@<tt>{fuel/core/config/asset.php}をコピーして使うのでそのファイルとの比較になります。一部、三項演算を使っていますが、pharでまとめない場合にもそのまま動くようにとの苦肉の策です。

//emlist{
-    'paths' => array('assets/'),
+    'paths' => array(DOCROOT . 'assets/'),
//}

//emlist{
-    'url' => Config::get('base_url'),
+    'url' => Config::get('base_url').(0===strpos(__DIR__,'phar://')?'index.phar/':''),
//}

//emlist{
-    'add_mtime' => true,
+    'add_mtime' => false,
//}


@<tt>{fuel/core/bootstrap.php}の変更部分です。パスの区切りの変更と関数の置き換えです。

//emlist{
-define('DS', DIRECTORY_SEPARATOR);
+define('DS', '/');

-defined('VENDORPATH') or define('VENDORPATH', realpath(COREPATH.'..'.DS.'vendor').DS);
+defined('VENDORPATH') or define('VENDORPATH', realpat_(COREPATH.'..'.DS.'vendor').DS);
//}


最後、@<tt>{fuel/core/classes/file/area.php}の変更部分です。

//emlist{
         {
-            $this->basedir = realpath($this->basedir) ?: $this->basedir;
+            $this->basedir = realpat_($this->basedir) ?: $this->basedir;
         }
//}

//emlist{
         {
-            $pathinfo['dirname'] = realpath($pathinfo['dirname']);
+            $pathinfo['dirname'] = realpat_($pathinfo['dirname']);
         }
         else
         {
             // attempt to get the realpath(), otherwise just use path with any double dots taken out when @<raw>{|latex|\n}basedir is set (for security)
-            $pathinfo['dirname'] = ( ! empty($this->basedir) ? realpath($this->basedir.DS.$pathinfo['dirname']) @<raw>{|latex|\n}: realpath($pathinfo['dirname']) )
+            $pathinfo['dirname'] = ( ! empty($this->basedir) ? realpat_($this->basedir.DS.$pathinfo['dirname']) @<raw>{|latex|\n}: realpat_($pathinfo['dirname']) )
                     ?: ( ! empty($this->basedir) ? $this->basedir.DS.str_replace('..', '', $pathinfo['dirname']) @<raw>{|latex|\n}: $pathinfo['dirname']);
//}


1つ1つ編集するのが大変であればGistの@<href>{https://gist.github.com/sharkpp/7716098,https://gist.github.com/sharkpp/7716098}に差分をアップしたので

//cmd{
$ cd fuelphp-1.7
$ wget -q https://gist.github.com/sharkpp/7716098/raw -O - | patch -u -p0
//}

//noindent
とすることで変更を適用することができます。


次は、pharの生成スクリプトです。

//emlist{
<?php
/*
 * Copyright (c) 2013 sharkpp
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 */
// 確実に削除 
@unlink('index.phar');
// phar書庫作成のためクラスを生成
$phar = new Phar(__DIR__ . '/index.phar', 0, 'index.phar');
// fuelphp1.7ディレクトリ丸ごと固める
$phar->buildFromDirectory(__DIR__ . '/fuelphp-1.7/');
// gzipで圧縮
//$phar->compressFiles(Phar::GZ); // ※ cssなどがうまく取り出せない
// 起動スタブを設定
$phar->setStub(<<<'EOD'
<?php
    function phar_rewrites($path) {
        if (0 === strpos($path,'/assets/'))
            return '/public' . $path;     // assetsだけはパスを変更
        return '/public/index.php'.$path; // あとはすべてindexに渡す
    }
    Phar::interceptFileFuncs();
    Phar::webPhar('index.phar', 'public/index.php', '', array(), 'phar_rewrites');
    __HALT_COMPILER(); ?>
EOD
);
//}


FuelPHPをインストールしたfuelphp-1.7ディレクトリの上にファイルを保存してください。


こちらもGistの@<href>{https://gist.github.com/sharkpp/7716423,https://gist.github.com/sharkpp/7716423}にアップしてあるので、

//cmd{
$ cd ~
$ wget -q https://gist.github.com/sharkpp/7716423/raw/mkphar.php
//}

//noindent
として、ローカルに保存できます。


準備ができたら

//cmd{
$ php mkphar.php
//}

//noindent
と入力して、index.pharを作成すると、70MBぐらいのファイルが出来上がります。


ドキュメントや.gitなどが含まれているので巨大になってしまいました。


@<strong>{ちなみに、Pharクラスでアーカイブを作成するには設定を変える必要があるかもしれません。}


具体的には、@<tt>{php.ini}の@<tt>{Phar}セクション内で@<tt>{phar.readonly = Off}と設定されている必要があります。

== ブラウザで確認


ここまでできたらindex.pharをウェブサーバーの公開フォルダに置きましょう。


と、その前に、AddTypeで.pharをphpで実行できるように@<tt>{.htaccess}を設置しましょう。

//emlist{
Options +FollowSymLinks
DirectoryIndex index.phar
AddType application/x-httpd-php .phar

<IfModule mod_rewrite.c>
    RewriteEngine on
    RewriteCond %{REQUEST_FILENAME} !-f
    RewriteCond %{REQUEST_FILENAME} !-d
    <IfModule mod_fcgid.c>
        RewriteRule ^(.*)$ index.phar?/$1 [QSA,L]
    </IfModule>
    <IfModule !mod_fcgid.c>
        <IfModule mod_php5.c>
            RewriteRule ^(.*)$ index.phar/$1 [L]
        </IfModule>
        <IfModule !mod_php5.c>
            RewriteRule ^(.*)$ index.phar?/$1 [QSA,L]
        </IfModule>
    </IfModule>
</IfModule>
//}


こちらも例によってGistの@<href>{https://gist.github.com/sharkpp/7718075,https://gist.github.com/sharkpp/7718075}にアップしてあるので、

//cmd{
$ wget -q https://gist.github.com/sharkpp/7718075/raw/.htaccess
//}

//noindent
で取得できます。


例えば、ローカルホストでウェブサーバーを動かしていてドキュメントルートに先の.htaccessと共に置いたのであれば、

//emlist{
http://127.0.0.1/
//}

//noindent
にブラウザでアクセスすると Welcome 画面が表示されます。

//emlist{
http://127.0.0.1/hello
//}

//noindent
にアクセスすると hello と表示されます。

//emlist{
http://127.0.0.1/xxxx
//}

//noindent
エラーページも表示できます。

== まとめ


お遊びのつもりで手を出してみたら、かなり時間をかけないとうまくいかなかったりで当てが外れてちょっとションボリ。


実際問題としてcoreの修正が必要となるので実用性となると皆無だと思います。


ただ、1ファイルでウェブサーバーにアプリが公開できるのは、うまく作れば面白いことが出来るのではないかとの期待が持てそうな機能でした。

//quote{
@<strong>{@sharkpp}


@TODO


Twitter: @<href>{https://twitter.com/sharkpp,@sharkpp}


Blog: @<href>{http://www.sharkpp.net/,http://www.sharkpp.net/}
//}
