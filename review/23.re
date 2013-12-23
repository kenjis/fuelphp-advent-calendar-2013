
= [FuelPHP]Heroku(PaaS)でFuelPHP環境(PHP5.3 + MySQL + Apatch)を構築する

== HerokuへFuelPHP環境を構築する手順をメモ


@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の23日目です。昨日は、@egmcさんの「@<href>{http://dasalog.eg2mix.com/fuel-advent2013-opauth/,FuelPHPがOpAuth対応になったのでfacebookログインをしてみる}」でした。


以前まではサービスをリリースするときにはレンタルサーバを借りてサービスをデプロイすることが一般的でしたが、最近はPaaSと呼ばれるアプリケーションの動作環境をプラットフォーム上で一式提供されている形態を使う事が増えてきています。有名どころではAmazonのAWSやMicrosoftのWindows Azureなどがあります。@<br>{}
 PaaSサービスのそれぞれの違いはスペック・料金・機能などがあり、レンタルサーバと比較した場合のPaaSのメリットはスケールアップ、アウトが簡単に行えることです。@<br>{}
 サービスのユーザ数が大幅にふえてサーバに負荷がかかるようになった場合や、予想してた以上にユーザ数が延びなかった場合などに役立ちます。


個人的にはHerokuはRailsのアプリをテスト的に公開したくなったときなどに使ったりしています。もともとHeroku自体はrubyの環境用としてスタートしていて現在はjava,node.js,ruby,phthonなどをサポートしています。@<br>{}
 現在Herokuではphpは非サポートとなっていますがphpも動作します。


Herokuコマンド参考ページ：@<href>{http://d.hatena.ne.jp/xyk/20101102,http://d.hatena.ne.jp/xyk/20101102}


buildpackは@<href>{https://github.com/winglian/heroku-buildpack-php,https://github.com/winglian/heroku-buildpack-php}を使う@<br>{}
 ————————————————————————————————–@<br>{}
 試みたのは、下記サイトに記載されているbuildpackを使用してサーバはnginxを使用するつもりでしたが、@<br>{}
 nginxのconfigファイルの設定がFuelPHPのプロジェクトに合わせるとなぜかうまくページが表示されず原因不明だったので今回は見送ることにします。すみません。@<br>{}
 @<href>{http://tkyk.name/blog/2012/11/28/php-on-heroku/,http://tkyk.name/blog/2012/11/28/php-on-heroku/}@<br>{}
 ————————————————————————————————–

=== 1.Herokuアカウントを取得


公式サイトより「login」押下して「signup」よりアカウントを取得します


[


//image[heroku][heroku]{
//}](http://to-developer.com/blog/?attachment_id=990)


公式サイト：@<href>{https://www.heroku.com/,https://www.heroku.com/}

=== 2.heroku toolbelt（ターミナルからherokuを操作するツール）をインストール


下記サイトよりtoolbeltを環境に合わせてインストールします


[


//image[2-300x163][2]{
//}](http://to-developer.com/blog/?attachment_id=589)


@<href>{https://toolbelt.heroku.com/%20,https://toolbelt.heroku.com/}

=== 3.SSH公開鍵の設定


(1)ターミナルを立ち上げてloginコマンドを実行

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ heroku login
Enter your Heroku credentials.
Email: [1で作成したアカウントを入力]
Password (typing will be hidden):  [1で作成したアカウントのパスワードを入力]
Authentication successful.
//}


「Authentication successful.」でアカウント認証成功　※「Authentication failed.」は失敗


(2)公開鍵をジェネレート

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{

Could not find an existing public key.
Would you like to generate one? [Yn] [Yを指定してエンター]
Generating new SSH public key.
Uploading SSH public key /Users/(PCユーザー名)/.ssh/id_rsa.pub
Authentication successful.
//}


「Authentication successful.」で成功


初期ログイン時に公開鍵を生成しなかった場合は手動で「ssh-keygen」で作成しherokuに公開鍵を設定する必要があります@<br>{}
 ssh-keygenについては前記事参照@<href>{http://to-developer.com/blog/?p=563,http://to-developer.com/blog/?p=563}


Herokuに公開鍵を設定する

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ heroku keys:add
//}


※Herokuのgui画面から設定も可

=== 4.fuelphpプロジェクトを生成


(1)アプリケーションを作成

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ oil create [アプリ名を入力]
//}


(2)index.php作成


Herokuはrootディレクトリにindex.phpがないと動作しないため、今のところ空のindex.phpファイルを生成しときます

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ touch index.php
//}


(3).htaccessを作成（fuelphpのディレクトリ構成に合わせてリダイレクト処理を入れる）@<br>{}
 fuelphpディレクトリ構成のpublic/以下にindex.phpのアクセスをリダイレクト@<br>{}
 参考サイト：@<href>{http://blog.livedoor.jp/erscape/archives/6937126.html,http://blog.livedoor.jp/erscape/archives/6937126.html}

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ vim .htaccess
//}


.htpaccessの設定情報

#@# lang: .brush: .php; .title: .; .notranslate title=""
//emlist{
  RewriteEngine on
  RewriteBase /
  RewriteRule ^(.+)-info\.php$ $1-info.php [L]
  RewriteCond %{SCRIPT_FILENAME} !^/app/www/public/
  RewriteRule ^(.*)$ public/$1 [L]
//}


(4)不要ファイル削除（サブモジュールなどはaddできないので削除）

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
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


※サブモジュールをaddする方法(git submodule addコマンド)@<br>{}
 例）opauthサブモジュールの場合

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ git submodule add git://github.com/andreoav/fuel-opauth.git fuel/packages/opauth
//}

=== 5.ローカルリポジトリにコミット

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ cd [アプリ名を入力]
$ git init
$ git commit -am "initial commit"
//}

=== 6.buildpackをherokuへインストール

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ heroku create --buildpack https://github.com/winglian/heroku-buildpack-php [アプリ名を入力]
//}


※アプリ名はここで入れなくてもデフォルトの名前が付けられる。gui画面などから確認・変更が可能

=== 7.herokuのリポジトリへ反映

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ git push heroku master
//}


※アプリが複数存在する場合、Herokuのリモートリポジトリが違いpushできない場合があるので都度確認が必要


※1 リモートリポジトリherokuの設定

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ git remote add heroku [リモートリポジトリ]
//}


※２ リモートリポジトリ確認

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ git remote show
//}


※3 リモートリポジトリ削除

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ git remote rm [リモートリポジトリ]
//}

=== 8.動作確認

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ heroku open
//}


fuelの画面がでたら成功！@<br>{}
 [


//image[3-1024x552][3]{
//}](http://to-developer.com/blog/?attachment_id=598)

=== 9.mysqlアドオンを入れる


無料版のアドオンを入れる場合も公式サイトからログインを行いクレジットカードの登録が必要です。@<br>{}
 ただ無料版の場合は料金が引かれるなど初期費用なども基本ないようです。


mysqlのアドオンは@<href>{https://addons.heroku.com/,https://addons.heroku.com/}からsearchボックスに「mysql」と検索すると2013/12時点で４つのアドオンが見つかりました。


Adminium Full fledged admin interface without touching your app code heroku addons:add adminium@<br>{}
 Amazon RDS Hook your app up to Amazon’s RDS heroku addons:add amazon_rds@<br>{}
 ClearDB MySQL Database The high speed, 100% uptime database for your MySQL powered applications. heroku addons:add cleardb@<br>{}
 Xeround Cloud Database αlpha Scalable, highly available, zero-management cloud database for MySQL heroku addons:add xeround


今回は「ClearDB」とする@<br>{}
 参考サイト：@<href>{http://www.ownway.info/Ruby/index.php?heroku%2Fhow%2Fmanagement%2Fdatabase%2Fcleardb,http://www.ownway.info/Ruby/index.php?heroku%2Fhow%2Fmanagement%2Fdatabase%2Fcleardb}


(1)公式サイトよりクレジットカード登録を行う


(2)アドオンをインストール

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ heroku addons:add cleardb:ignite
Adding cleardb:ignite on tranquil-cliffs-2547... done, v6 (free)
Use `heroku addons:docs cleardb:ignite` to view documentation.
//}


インストール完了


(3)接続情報を確認

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$ heroku config 
=== tranquil-cliffs-2547 Config Vars
BUILDPACK_URL:        https://github.com/winglian/heroku-buildpack-php
CLEARDB_DATABASE_URL: mysql://[ユーザ名]:[パスワード]@[ホスト名]/[DB名]?reconnect=true
//}


※CLEARDB_DATABASE_URLに接続文字列が表示@<br>{}
 CLEARDB_DATABASE_URL: mysql://[ユーザ名]:[パスワード]@[ホスト名]/[DB名]?reconnect=true


(4)接続文字列は？@<br>{}
 mysql –host=[ホスト名] –user=[ユーザ名] –password=[パスワード] [DB名]

=== 10.fuelphpプロジェクトからmysqlへ環境変数で接続

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//emlist{
$cleardb = parse_url(getenv('CLEARDB_DATABASE_URL'));
$conn = new PDO(
    sprintf("mysql:dbname=%s;host=%s", substr($cleardb['path'], 1), $cleardb['host']),
    $cleardb['user'],
    $cleardb['pass']
);
//}


@<strong>{ここまででphp + mysql + apatchのFuelPHPが動作する環境の構築完了です}

=== まとめ


Herokuにやや癖があり使いにくいと思ってしまった事もありましたが、慣れるとここまで行うのに15分程でできるとおもいます。一度環境を作ってしまえばあとの更新作業はすぐにできますね。@<br>{}
 他のPaaSはあまり使った事がありませんが、動作確認程度の使用でしたらHerokuでいいと思います。@<br>{}
 またHerokuから公式にphpがサポートされれば、もっとアドオン等増えてくるのではないでしょうか。


明日はクリスマスイブですね。いいことありますように！
