
= 続・Cloudn_PaaSでFuelPHPを動かしてみた


この記事は@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の１５日目です。


昨日は@<href>{https://twitter.com/sharkpp,@sharkpp}さんの@<href>{http://www.sharkpp.net/blog/2013/12/14/fuelphp-advent-calendar-2013-14th-day.html,「Request_Curlにまつわるエトセトラ」}でした。


本日の記事を担当する@<href>{https://twitter.com/Tukimikage,@Tukimikage}です。 この記事は以前公開した@<href>{http://think-sv.net/blog/?p=1075,「Cloud(n)PaaSでFuelPHPを動かしてみた」}をベースに中身をブラッシュアップしたものとなります。 尚、本記事のコードや各種コマンドは OS X 10.9 で動作確認しています。

== 1. Cloudn PaaSとは


NTTコミュニケーションズが提供する@<href>{http://www.ntt.com/cloudn/,Cloudn}のサービス・メニューの一つとして提供する@<href>{http://www.ntt.com/cloudn/data/paas.html,Platform as a Service}です。最小構成であれば１インスタンスあたり月額上限525円で利用可能です。

== 2. デプロイ環境の準備


前回の記事ではVMC(VMware Cloud CLI)を利用していましたが、今回は2013/09/26に公開された@<strong>{Cloudn PaaS UDN（コマンドラインツール）}を利用します。UDNはrubyが入っていればgemを利用してインストール可能です。

#@# lang: .brush: .powershell; .title: .; .notranslate title=""
//emlist{
$ sudo gem install udn 
Password:
Fetching: json_pure-1.6.8.gem (100%)
Successfully installed json_pure-1.6.8
Fetching: rubyzip-0.9.9.gem (100%)
Successfully installed rubyzip-0.9.9
 :
 :
Installing ri documentation for rb-readline-0.4.2
Parsing documentation for udn-0.3.23.3
Installing ri documentation for udn-0.3.23.3
10 gems installed
//}


インストールが完了したらログインしましょう。ログインに必要なメールアドレスとパスワードはCloudn PaaSの公式操作マニュアルをご覧ください。

#@# lang: .brush: .powershell; .title: .; .notranslate title=""
//emlist{
$ udn login
Attempting login to [http://api.cloudnpaas.com]
Email:
Password:
Successfully logged into [http://api.cloudnpaas.com]
//}

== 3. FuelPHPの開発環境準備


今回の開発環境準備には、@<href>{https://twitter.com/chatii0079,@chatii0079}さんが作成されたfuelphp_setupを利用します。fuelphp_setupについては、@<href>{http://chatii.net/articles/php/2013/12/fuelphp-composer.html,FuelPHP Advent Calendar 2013 11日目「FuelPHP をもっと Composer で使う」}をご覧ください。


Cloudn PaaSは2013/12/15現在、PHP v5.3に対応していますが、PHPフレームワークのフレームワークには全く対応していません。（個人的にはかなり残念ですが・・・）


FuelPHPを利用する場合はDocumentRootの変更が必要になります。 ユーザ設置の.htaccessが動作するため、FuelPHPプロジェクトのディレクトリトップに以下の通りファイルを作成します。

#@# lang: .brush: .powershell; .title: .; .notranslate title=""
//emlist{
RewriteEngine On
RewriteBase /
RewriteRule ^(.*)$ /public/index.php [L]
//}


@<href>{https://twitter.com/chatii0079,@chatii0079}さんがGithubで公開されている@<href>{https://github.com/chatii/fuelphp_setup,fuelphp_setup}をForkして、.htaccessファイルを追加した@<href>{https://github.com/Y-NAKA/fuelphp_setup_for_cloudn_paas,fuelphp_setup_for_cloudn_paas}を作成させていただきました。@<href>{https://twitter.com/chatii0079,@chatii0079}さん、ありがとうございます！


Githubからのプロジェクト作成方法は、@<href>{https://github.com/chatii/fuelphp_setup,fuelphp_setup}の手順と同様です。

#@# lang: .brush: .powershell; .title: .; .notranslate title=""
//emlist{
$ git clone https://github.com/Y-NAKA/fuelphp_setup_for_cloudn_paas.git project_name
$ cd project_name
$ rm -rf ./.git/
$ git init
$ curl -sS https://getcomposer.org/installer | php
$ php composer.phar install
//}

== 4. テストアプリ作成


今回は動作確認のためにテスト用クラスを生成します。（ディレクトリ構成はダミーです）

#@# lang: .brush: .powershell; .title: .; .notranslate title=""
//emlist{
$ oil g controller main index
    Creating view: /project_name/fuel/app/views/template.php
    Creating view: /project_name/fuel/app/views/main/index.php
    Creating controller: /project_name/fuel/app/classes/controller/main.php
//}


テスト用クラスを読み込むためにルーティング設定を変更します。 @<href>{https://github.com/chatii/fuelphp_setup,fuelphp_setup}を利用して環境構築した場合でも、welcomeコントローラーが存在する前提のルーティングになっているため、必要ないところは削除しrootのルーティング先を書き換えます。


ファイル名： /project_name/fuel/app/config/routes.php

#@# lang: .prettyprint .linenums:1
//emlist{
<?php
return array(
    '_root_'  => 'main/index',  // The default route
);
//}


 oilで生成されたテンプレートファイルにはbootstrap.cssをインポートする記述があるため、publicディレクトリに必要なサブディレクトリとファイルを設置します。 


//image[b7babaee0c0a17793fc07f21303a83e2][bootstrap.cssを設置]{
//}

== 5. デプロイ


では、早速デプロイしていきます。以下の例では @<strong>{testapp1} という名前でアプリケーションを作成しています。


ポイントは @<strong>{Detected a Standalone Application, is this correct?} という質問に @<strong>{No} で答えて、次の言語／FW選択で @<strong>{9: PHP} を選ぶことです。 Deployed URLやインスタンスに割り当てる性能要件などは自由に変更して頂いて構いません。

#@# lang: .brush: .powershell; .title: .; .notranslate title=""
//emlist{
$ cd project_name
$ udn push testapp1
Would you like to deploy from the current directory? [Yn]: 
Detected a Standalone Application, is this correct? [Yn]: n
1: Rails
2: Spring
3: Grails
4: Lift
5: JavaWeb
6: Standalone
7: Sinatra
8: Node
9: PHP
10: WSGI
11: Django
12: Rack
13: Play
Select Application Type: 9
Selected PHP Application
Application Deployed URL [testapp1.cloudnpaas.com]: 
Memory reservation (128M, 256M, 512M, 1G, 2G) [128M]: 
How many instances? [1]: 
Bind existing services to 'testapp1'? [yN]: 
Create services to bind to 'testapp1'? [yN]: 
Would you like to save this configuration? [yN]: 
Creating Application: OK
Uploading Application:
  Checking for available resources: OK
  Processing resources: OK
  Packing application: OK
  Uploading (8M): OK
Push Status: OK
Staging Application 'testapp1': OK
Starting Application 'testapp1': OK
//}

== 6. 動作確認


デプロイ後はブラウザから動作確認を行ってください。 


//image[9fa207a0e60ca2e0f8c2b565923fa67d][動作確認]{
//}


余談ですが、Cloudn PaaS含め、Cloudnの様々な機能をWebからコントールできる管理UIは、Firefoxが推奨ブラウザとなっています。Chromeなど他のブラウザで正常に動かない機能があるので注意してください。

== 7. アプリケーションのアクセス制限


開発中のアプリや一般公開する必要が無いアプリはIPアドレスでアクセス制限をかけることができます。尚、当ブログのコードハイライトの関係で以下のサンプルは、実際の見た目とは異なる場合があります。

 * アクセス制限設定（IPアドレスはCIDR記法で記載し、カンマで区切ることで複数指定可能）

#@# lang: .brush: .powershell; .title: .; .notranslate title=""
//emlist{
$ udn env-add testapp1 ALLOW_CIDR_WHITELIST="＊．＊．＊．＊／＊"
Adding Environment Variable [ALLOW_CIDR_WHITELIST=＊．＊．＊．＊／＊]: OK
Stopping Application 'testapp1': OK
Staging Application 'testapp1': OK
Starting Application 'testapp1': OK
//}
 * アクセス制限状況確認

#@# lang: .brush: .powershell; .title: .; .notranslate title=""
//emlist{
$ udn env testapp1
ALLOW_CIDR_WHITELIST ＊．＊．＊．＊／＊
//}
 * アクセス制限解除（一度登録した内容を変更する場合は一度解除した上で新規設定）

#@# lang: .brush: .powershell; .title: .; .notranslate title=""
//emlist{
$ udn env-del testapp1 ALLOW_CIDR_WHITELIST
Deleting Environment Variable [ALLOW_CIDR_WHITELIST]: OK
Stopping Application 'testapp1': OK
Staging Application 'testapp1': OK
Starting Application 'testapp1': OK
//}



アクセス制限中は以下のように表示されます。 


//image[dfeb886dca608cf1cdb7c1246bf8daee][アクセス制限]{
//}

=== アクセス制限に関する注意事項


.htaccess にてIPアドレス制限やBasic認証を行うこともできますが、Cloudn PaaSのサーバは、ユーザアプリが動作するアプリサーバと、インターネットからのアクセスを受け付けるプロキシサーバという2段構えになっているようです。そのため、アプリサーバ上の.htaccessで単純にアクセス制限すると正常に動かなくなります。

== 8. アプリケーションの更新


アプリの更新は更新したファイルが格納されているディレクトリにて、以下の通りコマンドを実行します。 ファイルの更新とアプリの再起動を行ってくれます。

#@# lang: .brush: .powershell; .title: .; .notranslate title=""
//emlist{
$ cd project_name
$ udn update testapp1
Uploading Application:
  Checking for available resources: OK
  Processing resources: OK
  Packing application: OK
  Uploading (92K): OK
Push Status: OK
Stopping Application 'testapp1': OK
Staging Application 'testapp1': OK
Starting Application 'testapp1': OK
//}

== 9. おわりに


今回はCloudn PaaSにてFuelPHPを動かす方法を、前回記事にした部分からのアップデートを交えて解説しました。


FuelPHPの開発環境自体も、@<href>{https://twitter.com/chatii0079,@chatii0079}さんの@<href>{https://github.com/chatii/fuelphp_setup,fuelphp_setup}を利用することでかなり簡単に整えることができます。 大規模場システムを作る場合は役不足な感じがあるPaaSですが、ちょこっとツールを作りたいとか、ちょこっと検証してみたいとか、利用シーンによってはかなり便利に使えるのではないでしょうか。


FuelPHP☓Cloudnに関しては今後も記事を書いていきたいと思いますので、ご期待ください。


明日は@<href>{https://twitter.com/hosopy,@hosopy}さんの@<href>{#,「moduleを使いこなす」}です。 お楽しみに！
