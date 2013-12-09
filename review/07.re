
= FuelPHP開発でローカルとWebで構造が変わっても対応できる小技


こんにちはタケ（@<href>{https://twitter.com/LandscapeSketch,@LandscapeSketch}）です。


今日は「@<strong>{FuelPHP開発でローカルとWebで構造が変わっても対応できる小技}」として、ローカルとWeb上でディレクトリ構造を変えていても便利に開発できる小技を紹介します。

== 開発環境とWeb上の構造


現在、ローカルではWindows、XAMPP、NetBeansの組み合わせで開発しています。@<br>{}
ローカルでは1つのプロジェクトディレクトリに全てを入れ管理を行います。


//image[fuelphp-adv-2-500x336][ローカル開発環境]{
//}


Web上では安全のためにfuelphp/publicの中身のみを公開ディレクトリに入れ、fuelphp/fuelは非公開エリアに移動しようと思いました。


//image[fuelphp-adv-1][オンライン公開状態]{
//}


しかしNetBeansでは@<strong>{プロジェクトディレクトリ以下に入っているファイルしか管理できません}。サイト名のメインディレクトリがあり、その下にfuel、public両方が入っている必要があります（@<img>{fuelphp-adv-2-500x336}の点線部分）。


Webと同じ構造にするとNetBeansで管理できなくなってしまうのです。


となると@<strong>{デプロイするたびに設定を書き換えてアップロード}する必要性が出てきます。手間がかかり、もし書き忘れた場合はエラー画面となってしまいます。

== index.phpを柔軟にする


いろいろ試した結果、public/index.phpを書き換えることで良い感じになりました。


public/index.phpを開くとFuelPHPで使用するディレクトリパスの設定部分があります。


通常では

//emlist{
define('APPPATH', realpath(DIR . '/../fuel/app/' ).DIRECTORY_SEPARATOR);
//}


のように/../fuel/app/の部分が固定されています。


まずこの部分を変数に置き換えます。 /../ を $fueldir に定義しています。

//emlist{
define('APPPATH', realpath(DIR . $fueldir . '/app/').DIRECTORY_SEPARATOR);
//}

== .htaccessに仕掛け


次にそれぞれの環境の.htaccessにfuelの環境設定を書き加えます。

//emlist[ローカル]{
SetEnv FUEL_ENV development
//}

//emlist[Web]{
SetEnv FUEL_ENV production
//}

== 再度index.php


再度public/index.phpに以下の構文を加えました。

//emlist{
if ($_SERVER['FUEL_ENV'] == 'production') {
    $fuel_dir = '/../../fuel';
    ini_set('display_errors', 0);
} elseif ($_SERVER['FUEL_ENV'] == 'development') {
    $fuel_dir = '/../fuel';
    ini_set('display_errors', 1);
}
//}


$fuel_dirに環境ごとのfuelディレクトリ位置を代入しています。その後は各グローバル変数に代入されていきます。
また同時に

//emlist{
iniset('displayerrors', 0);
//}


などエラー表示設定を書いておくことで、公開した時に内部エラーが丸見えにならないようにもできました。

== まとめ


ごく簡単なTipsでしたがいかがでしたでしょうか？


ただこれは私の考えた方法で、もっと簡潔で安全な方法もあるかと思います。もし「こんなふうに書くともっと簡単だよ！」という方法がありましたら、@<strong>{ぜひコメントしてください！}


あすは@sa2yasuさんの「FuelPHPを更に使ってみて使えるなと思ったCustomValidationRuleの書き方とCore拡張の小技」です。お楽しみに～


（文中のソースコードはすべてBSDライセンスといたします。）

//quote{
@<strong>{@LandscapeSketch}

@TODO

Twitter: @<href>{https://twitter.com/LandscapeSketch,@LandscapeSketch}

Blog: @<href>{http://worktoolsmith.com/,http://worktoolsmith.com/}
//}
