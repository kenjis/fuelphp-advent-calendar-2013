
= FuelPHPの開発環境を20分で構築する（Vagrant編）


@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の2日目です。@<br>{}


従来は、FuelPHPの開発環境を構築する場合、XAMPPやMAMPを使う方法が一般的でした。この方法は簡単に手許のPCに開発環境を構築でき便利なのですが、開発環境と本番環境のPHPのバージョンが異なったり、開発環境はWindowsやMacだが本番環境はLinuxであったりと、ほとんどの場合、本番環境と開発環境が異なるという問題がありました。


PHPのポータビリティはかなり高いので、多くの場合、実際には問題は生じませんが、ファイル名の大文字小文字の違いやパーミッション、PHPのバグなど、本番環境だけで問題が発生するということも可能性としてはあります。


この問題を解消するには、開発環境と本番環境をできる限り一致させることが望ましいです。そこで、本番と同じような仮想マシンを作成し、そこで開発をすれば開発環境と本番環境をほとんど一致させることが可能になります。


本日は、VirtualBoxとVagrantを使って、FuelPHPのためのそのような開発環境を簡単に作成する方法を解説します。


なお、この方法のデメリットは、仮想マシンを維持するためのリソースが余計に必要となることです。OSからまるごとインストールするわけですから、ハードディクスの容量もその分多く必要になりますし、仮想マシン実行のオーバーヘッドがありますので、実効速度もネイティブで動作しているWebサーバ／PHPより、多くの場合、遅くなるでしょう。

== VirtualBoxとVagrantのインストール


フリーな仮想化ソフトウェアであるVirtualBoxを、以下からダウンロードしインストールします（執筆時の動作確認バージョンはVirtualBox 4.3.2）。

 * @<href>{https://www.virtualbox.org/wiki/Downloads,https://www.virtualbox.org/wiki/Downloads}



VirtualBoxの仮想イメージを操作するツールであるVagrantを、以下からダウンロードしインストールします（執筆時の動作確認バージョンはVagrant 1.3.5）。

 * @<href>{http://downloads.vagrantup.com/,http://downloads.vagrantup.com/}



これで準備は完了です。

== 仮想マシンの作成


FuelPHPのプロジェクト用のフォルダを作成し、その中にVagrant用のファイルを配置します。

//cmd{
$ mkdir fuelphp
$ cd fuelphp/
$ git clone git@github.com:kenjis/vagrant-fuelphp-centos6.git
$ cd vagrant-fuelphp-centos6/
$ git submodule update --init --recursive
//}


仮想マシンを構築します。

//cmd{
$ vagrant up
//}


初回はCentOS6の仮想イメージをダウンロードするため、かなり時間がかかりますので気長に待ちます（このダウンロード時間を含めると20分で開発環境を構築するのは無理です）。


これで、仮想マシンが作成され、FuelPHPの開発に必要なサーバなどもインストール設定されます。


FuelPHPがインストールされていない場合は、@<tt>{oil create}コマンドでインストールされます。


これで、@<href>{http://localhost:8000/,http://localhost:8000/}にアクセスすれば、おなじみのFuelPHPのWelcomeページが表示されます。

== ディレクトリ構成

//noindent
ホスト（手許のPC）側

//emlist{
fuelphp/（FuelPHPプロジェクトのトップ）
├── docs
├── fuel
├── public
└── vagrant-fuelphp-centos6
//}

//noindent
ゲスト（仮想マシン）側

//emlist{
/mnt/fuelphp/
├── docs
├── fuel
├── public
└── vagrant-fuelphp-centos6
//}


仮想マシンから、ホスト側のfuelphpフォルダを共有しているので、ホスト側から好きなエディタでソースを変更すれば、仮想マシンに自動的に反映されます。

== テストの実行


vagrant-fuelphp-centos6フォルダから、@<tt>{vagrant ssh}コマンドで仮想マシンにSSHで接続できます（Windowsを除く）。

//cmd{
$ cd fuelphp/vagrant-fuelphp-centos6/
$ vagrant ssh
Last login: Mon Dec  2 01:11:37 2013 from 10.0.2.2
Welcome to your Vagrant-built virtual machine.
//}


ホームディレクトリにシンボリックリンクが張ってあるので、@<tt>{/mnt/fuelphp}フォルダには@<tt>{~/fuelphp}でアクセスできます。


oilコマンドとphpunitもインストール済みなのですぐに実行できます。

//cmd{
[vagrant@localhost ~]$ cd fuelphp/
[vagrant@localhost fuelphp]$ oil test --group=Core
Tests Running...This may take a few moments.
PHPUnit 3.7.28 by Sebastian Bergmann.

Configuration read from /mnt/fuelphp/fuel/core/phpunit.xml

...............................................................  63 / 361 ( 17%)
............................................................... 126 / 361 ( 34%)
............................................................... 189 / 361 ( 52%)
............................................................... 252 / 361 ( 69%)
............................................................... 315 / 361 ( 87%)
..............................................

Time: 7.92 seconds, Memory: 19.25Mb

OK (361 tests, 413 assertions)
//}

== サーバ環境


vagrant-fuelphp-centos6で作成される仮想マシンのサーバ環境は以下のようになっています。

 * メモリ 480MB
 * HDD 200GB
 * OS CentOS 6.4 (64bit)
 * Apache 2.2.15-29.el6.centos.x86_64
 * MySQL 5.1.69-1.el6_4.x86_64
 * PHP 5.4.21-2.ius.el6.x86_64
 * phpMyAdmin 3.5.8.2-1.el6.noarch
 * PHPUnit 3.7.28



ホスト側のポート8000が仮想マシンのポート80に転送されるようになっています。仮想マシンに直接アクセスする場合は、@<href>{http://192.168.33.33/,http://192.168.33.33/}にアクセスします。


ホストのFuelPHPのプロジェクトのフォルダが仮想マシンの@<tt>{/mnt/fuelphp}にマウントされるようになっています。


MySQLデータベースは、@<tt>{fuel_dev}と@<tt>{fuel_test}が作成されており、rootのパスワードは@<tt>{root}です。


また、@<href>{http://localhost:8000/phpmyadmin/,http://localhost:8000/phpmyadmin/}から、phpMyAdminにアクセスできます。

== 仮想マシンの起動と停止


仮想マシンの停止は、vagrant-fuelphp-centos6フォルダに移動して、

//cmd{
$ vagrant halt
//}

//noindent
とします。@<tt>{vagrant suspend}コマンドを実行すれば、仮想マシンをシャットダウンせずに状態を保存したまま停止できます。


仮想マシンの起動は、

//cmd{
$ vagrant up
//}


仮想マシンを破棄するには、

//cmd{
$ vagrant destroy
//}

//noindent
とします。

== その他のVagrant環境


今回の開発環境はCentOS 6.4を使ったものですが、Ubuntu 12.04（precise64）の場合は、以下のvagrant-fuelphpが公開されています。

 * @<href>{https://github.com/iturgeon/vagrant-fuelphp,https://github.com/iturgeon/vagrant-fuelphp}
 * @<href>{http://blog.a-way-out.net/blog/2013/11/13/fuelphp-vagrant-setup/,vagrant-fuelphpを使ってFuelPHPの開発環境を構築する}



また、PHPの開発環境として以下にまとめがあり、参考になります。

 * @<href>{http://www.engineyard.co.jp/blog/2013/vagrantfile-for-php/,PHPの開発に使えるVagrantfileのまとめ}


== 関連

 * @<href>{http://blog.a-way-out.net/blog/2013/11/15/fuelphp-vagrant-centos/,Vagrantを使ってFuelPHPの開発用のCentOS 6.4を構築する}
 * @<href>{http://blog.a-way-out.net/blog/2013/11/20/vagrant-vb-gui-mode/,VagrantでVirtualBoxの仮想マシンのウィンドウを表示させる}


//quote{
@<strong>{kenjis}


FuelPHPまとめWiki管理人。「PHP5 技術者認定上級試験」認定者。


Twitter: @<href>{https://twitter.com/kenji_s,@kenji_s}


Blog: @<href>{http://blog.a-way-out.net/,http://blog.a-way-out.net/}
//}
