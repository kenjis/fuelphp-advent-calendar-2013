= Heroku（PaaS）でFuelPHP環境（PHP 5.3＋MySQL＋Apache）を構築する

== HerokuへFuelPHP環境を構築する手順をメモ

@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の23日目です。

以前まではサービスをリリースするときにはレンタルサーバを借りてサービスをデプロイすることが一般的でしたが、最近はPaaSと呼ばれるアプリケーションの動作環境をプラットフォーム上で一式提供されている形態を使う事が増えてきています。有名どころではAmazonのAWSやMicrosoftのWindows Azureなどがあります。

PaaSサービスのそれぞれの違いはスペック・料金・機能などであり、レンタルサーバと比較した場合のPaaSのメリットはスケールアップ、アウトが簡単に行えることです。

サービスのユーザ数が大幅にふえてサーバに負荷がかかるようになった場合や、予想してた以上にユーザ数が延びなかった場合などに役立ちます。@<br>{}

個人的にはHerokuはRailsのアプリをテスト的に公開したくなったときなどに使ったりしています。もともとHeroku自体はRubyの環境用としてスタートしていて現在はJava、node.js、Ruby、Pythonなどをサポートしています。@<br>{}

現在HerokuではPHPは非サポートとなっていますがPHPも動作します。

Buildpackは@<href>{https://github.com/winglian/heroku-buildpack-php,https://github.com/winglian/heroku-buildpack-php}を使います。

//note[]{
下記サイトに記載されているBuildpackを使用してサーバはnginxを使用するつもりでしたが、nginxのconfigファイルの設定をFuelPHPのプロジェクトに合わせると、なぜかうまくページが表示されず原因不明だったので今回は見送ることにします。すみません。@<br>{}
@<href>{http://tkyk.name/blog/2012/11/28/php-on-heroku/,http://tkyk.name/blog/2012/11/28/php-on-heroku/}
//}

 * Herokuコマンド参考ページ：@<href>{http://d.hatena.ne.jp/xyk/20101102,http://d.hatena.ne.jp/xyk/20101102}@<br>{}

=== 1. Herokuアカウントを取得

公式サイトより「login」押下して「signup」よりアカウントを取得します。

 * Heroku公式サイト：@<href>{https://www.heroku.com/,https://www.heroku.com/}

//image[heroku][Heroku]{
//}

=== 2. Heroku Toolbelt（ターミナルからHerokuを操作するツール）をインストール

下記サイトよりHeroku Toolbeltを環境に合わせてインストールします。

 * @<href>{https://toolbelt.heroku.com/,https://toolbelt.heroku.com/}

//image[2-300x163][Heroku Toolbelt]{
//}]

=== 3. SSH公開鍵の設定

==== (1) ターミナルを立ち上げてloginコマンドを実行

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ heroku login
Enter your Heroku credentials.
Email: [1で作成したアカウントを入力]
Password (typing will be hidden):  [1で作成したアカウントのパスワードを入力]
Authentication successful.
//}

「Authentication successful.」でアカウント認証成功、「Authentication failed.」は失敗です。

==== (2) 公開鍵をジェネレート

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{

Could not find an existing public key.
Would you like to generate one? [Yn] [Yを指定してエンター]
Generating new SSH public key.
Uploading SSH public key /Users/(PCユーザー名)/.ssh/id_rsa.pub
Authentication successful.
//}

「Authentication successful.」で成功です。

初期ログイン時に公開鍵を生成しなかった場合は手動で「ssh-keygen」で作成しHerokuに公開鍵を設定する必要があります。

ssh-keygenについては「ssh-keygenコマンドで秘密鍵・公開鍵生成（@<href>{http://to-developer.com/blog/?p=563,http://to-developer.com/blog/?p=563}）」を参照してください。

==== (3) Herokuに公開鍵を設定

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ heroku keys:add
//}

HerokuのGUI画面から設定も可能です。

=== 4. FuelPHPプロジェクトを生成

==== (1) アプリケーションを作成

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ oil create [アプリ名を入力]
//}

==== (2) index.phpの作成

Herokuはrootディレクトリにindex.phpがないと動作しないため、今のところ空のindex.phpファイルを生成しときます。

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ touch index.php
//}

==== (3) .htaccessを作成（FuelPHPのディレクトリ構成に合わせてリダイレクト処理を入れる）

FuelPHPディレクトリ構成のpublic/以下にindex.phpのアクセスをリダイレクトします。@<br>{}

参考サイト：@<href>{http://blog.livedoor.jp/erscape/archives/6937126.html,http://blog.livedoor.jp/erscape/archives/6937126.html}

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ vim .htaccess
//}

#@# lang: .brush: .php; .title: .; .notranslate title=""
//emlist[.htaccessの設定情報]{
  RewriteEngine on
  RewriteBase /
  RewriteRule ^(.+)-info\.php$ $1-info.php [L]
  RewriteCond %{SCRIPT_FILENAME} !^/app/www/public/
  RewriteRule ^(.*)$ public/$1 [L]
//}

==== (4) 不要ファイル削除（サブモジュールなどはaddできないので削除）

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ rm -rf .git .gitmodules
$ rm *.md
$ rm -rf docs
// 下記も不要なため削除
$ rm -fr fuel/core/
$ rm -fr fuel/packages/auth/
$ rm -fr fuel/packages/email/
$ rm -fr fuel/packages/oil/
$ rm -fr fuel/packages/orm/
$ rm -fr fuel/packages/parser/
//}

==== サブモジュールをaddする方法（git submodule addコマンド）

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd[（例）opauthサブモジュールの場合]{
$ git submodule add git://github.com/andreoav/fuel-opauth.git fuel/packages/opauth
//}

=== 5. ローカルリポジトリにコミット

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ cd [アプリ名を入力]
$ git init
$ git commit -am "initial commit"
//}

=== 6. buildpackをHerokuへインストール

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ heroku create --buildpack https://github.com/winglian/heroku-buildpack-php [アプリ名を入力]
//}

アプリ名はここで入れなくてもデフォルトの名前が付けられます。GUI画面などから確認・変更が可能です。

=== 7. Herokuのリポジトリへ反映

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ git push heroku master
//}

アプリが複数存在する場合、Herokuのリモートリポジトリが違いpushできない場合があるので都度確認が必要です。

==== 1. リモートリポジトリherokuの設定

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ git remote add heroku [リモートリポジトリ]
//}

==== 2. リモートリポジトリ確認

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ git remote show
//}

==== 3. リモートリポジトリ削除

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ git remote rm [リモートリポジトリ]
//}

=== 8. 動作確認

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ heroku open
//}

FuelのWelcome画面がでたら成功！

//image[3-1024x552][Welcome画面]{
//}

=== 9. MySQLアドオンを入れる

無料版のアドオンを入れる場合も公式サイトからログインを行いクレジットカードの登録が必要です。
ただ無料版の場合は料金が引かれるなど初期費用なども基本ないようです。

MySQLのアドオンは@<href>{https://addons.heroku.com/,https://addons.heroku.com/}からsearchボックスに「mysql」と検索すると2013/12時点で4つ見つかりました。

 * Adminium Full fledged admin interface without touching your app code heroku addons:add adminium
 * Amazon RDS Hook your app up to Amazon’s RDS heroku addons:add amazon_rds
 * ClearDB MySQL Database The high speed, 100% uptime database for your MySQL powered applications. heroku addons:add cleardb
 * Xeround Cloud Database αlpha Scalable, highly available, zero-management cloud database for MySQL heroku addons:add xeround

今回は「ClearDB」を使用します。

 * 参考サイト：@<href>{http://www.ownway.info/Ruby/index.php?heroku%2Fhow%2Fmanagement%2Fdatabase%2Fcleardb,http://www.ownway.info/Ruby/index.php?heroku%2Fhow%2Fmanagement%2Fdatabase%2Fcleardb}

==== (1) 公式サイトよりクレジットカード登録を行う

==== (2) アドオンをインストール

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ heroku addons:add cleardb:ignite
Adding cleardb:ignite on tranquil-cliffs-2547... done, v6 (free)
Use `heroku addons:docs cleardb:ignite` to view documentation.
//}

インストール完了です。

==== (3) 接続情報を確認

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
$ heroku config 
=== tranquil-cliffs-2547 Config Vars
BUILDPACK_URL:        https://github.com/winglian/heroku-buildpack-php
CLEARDB_DATABASE_URL: mysql://[ユーザ名]:[パスワード]@[ホスト名]/[DB名]?reconnect=true
//}

CLEARDB_DATABASE_URLに接続文字列が表示されます。

//emlist{
CLEARDB_DATABASE_URL: mysql://[ユーザ名]:[パスワード]@[ホスト名]/[DB名]?reconnect=true
//}

==== (4) 接続文字列は？

//emlist{
mysql –host=[ホスト名] –user=[ユーザ名] –password=[パスワード] [DB名]
//}

=== 10. FuelPHPプロジェクトからMySQLへ環境変数で接続

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$cleardb = parse_url(getenv('CLEARDB_DATABASE_URL'));
$conn = new PDO(
    sprintf("mysql:dbname=%s;host=%s", substr($cleardb['path'], 1), $cleardb['host']),
    $cleardb['user'],
    $cleardb['pass']
);
//}

@<strong>{ここまででPHP＋MySQL＋ApacheでFuelPHPが動作する環境の構築完了です}。

=== まとめ

Herokuにやや癖があり使いにくいと思ってしまった事もありましたが、慣れるとここまで行うのに15分程でできるとおもいます。一度環境を作ってしまえばあとの更新作業はすぐにできますね。
他のPaaSはあまり使った事がありませんが、動作確認程度の使用でしたらHerokuでいいと思います。
またHerokuから公式にphpがサポートされれば、もっとアドオン等増えてくるのではないでしょうか。

明日はクリスマスイブですね。いいことありますように！

//quote{
@<strong>{@mycb750}

@TODO

Twitter: @<href>{https://twitter.com/mycb750,@mycb750}

Blog: @<href>{http://to-developer.com/blog/,http://to-developer.com/blog/}
//}
