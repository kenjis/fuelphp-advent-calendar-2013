= イベント機能を使ってアプリケーションをカスタマイズする

@<href>{http://atnd.org/events/45096,FuelPHP advent Calendar 2013}の10日目です。

今日は、「イベント機能を使ってアプリケーションをカスタマイズする」です。

== イベントとは何か

イベントとは、FuelPHPコアを書き換えることなく、独自の処理を差し込むことができる仕組みです。

 * FuelPHPの処理の途中に、トリガー(ここを通ったときに、追加処理を実行する場所)がいくつか用意されている
 * トリガー名を指定して、独自の処理を追加する
 * 独自の処理が追加されたトリガーのところで、独自の処理が実行される

という仕組みです。WordPressを使っている方であれば、「WordPressのアクションフックに似ている」と思うかもしれません。

//image[fuelphp_event1][アプリケーションの処理の流れ：青丸がトリガー]{
//}

//image[fuelphp_event2][イベントを実行：処理途中の各トリガーの所では、登録されているイベントがあれば実行する]{
//}

//image[fuelphp_event3][コードは点線部分で分離されている：アプリケーション自体のコードに手を入れずに、機能追加／変更する]{
//}

== 独自の処理を追加する

2通りの方法が用意されています。

 1. @<tt>{app/config/event.php}に追加する方法
 1. @<tt>{Event::register}を使う方法

1.は、@<tt>{app/config}に@<tt>{event.php}に書く方法です。詳細は@<href>{http://fuelphp.jp/docs/1.7/classes/event.html,Eventクラス（FuelPHP 1.7）}をごらんください。

2.は、@<tt>{register}メソッドを使う方法です。今回はこちらを使います。@<href>{http://fuelphp.jp/docs/1.7/classes/event.html,Eventクラス（FuelPHP 1.7）}では、@<tt>{user_login}に@<tt>{Class::method}処理を追加する例が掲載されています。

#@# lang: .brush: .php; .title: .; .notranslate title=""
//emlist{
Event::register('user_login', 'Class::method');
//}

@<tt>{bootstrap.php}に書けば、トリガーにフックできます。

== トリガーにフックしてみる

では、実際にトリガーにフックしてみましょう。ここでは、@<href>{http://novius-os.jp/,Novius OS}にフックする例を紹介します。Novius OSは、FuelPHPベースのCMSで、jQuery UI、Wijmoを使った使い易いインターフェースが特徴です。@<href>{http://docs-api.novius-os.org/en/chiba.2/php/events.html,Novius OS Chiba2のイベント一覧}にある@<tt>{admin.loginFail}を使い、ログイン失敗をログに記録します。

#@# lang: .brush: .php; .title: .; .notranslate title=""
//emlist{
/**
 * Copyright (c) 2013 Fumito MIZUNO
 * License: MIT 
 * http://opensource.org/licenses/mit-license.php
 */
Event::register('admin.loginFail', 'warning_on_loginfail');

function warning_on_loginfail()
{
    $message = 'Login Fail.';
    $message .= '  Email: ' . Input::post('email');
    $message .= '  Password: ' . Input::post('password');
    $message .= '  IP: ' . Input::ip();
    Log::warning($message);
}
//}

Novius OSは、メールアドレスとパスワードでユーザー認証するので、ログイン失敗時にそれらを記録します。これらはFuelPHPの@<tt>{Input}クラスを利用します。引数無しで@<tt>{Input::post()}を実行すると@<tt>{$_POST}を全部取ってきて配列で返すので、丸ごと記録したければ@<tt>{Format::forge(Input::post())->to_serialized()}としてもいいでしょう。またアクセス元のIPアドレスも記録し、ログ記録にはFuelPHPの@<tt>{Log}クラスを利用します。ログ記録の内部処理は@<href>{https://github.com/Seldaek/monolog,Monolog}ライブラリ（MITライセンス）が行っています。

ログインに失敗すると、ログファイルに

//emlist{
WARNING - 2013-12-09 09:15:23 --> Login Fail.  Email: email@example.com  Password: p@ssw0rd  IP: 127.0.0.1
//}

のように記録されます。

== デフォルトで用意されているイベント

@<href>{http://fuelphp.jp/docs/1.7/classes/event.html,Event クラス（FuelPHP 1.7）}によると、デフォルトで用意されているイベントは、@<tt>{app_created}、@<tt>{request_created}、@<tt>{request_started}、@<tt>{controller_started}、@<tt>{controller_finished}、@<tt>{response_created}、@<tt>{request_finished}、@<tt>{shutdown}です。これらのトリガーに処理を追加することができます。

== アプリケーションにトリガーを用意する

コアに用意されているトリガーを使うだけではなく、アプリケーションにもトリガーを用意することができます。@<img>{fuelphp_event1}の、青丸を作る作業です。コード中の適当な箇所に、@<tt>{Event::trigger(トリガー名)}と書けばOKです。

Novius OSのログイン部分のコード（@<href>{https://github.com/novius-os/core/blob/master/chiba2/framework/classes/controller/admin/login.ctrl.php,novius-os/framework/classes/controller/admin/login.ctrl.php}）を見てみると、ログイン成功時に@<tt>{admin.loginSuccess}イベント、ログイン失敗時に@<tt>{admin.loginFail}イベント、が用意されています。

#@# lang: .brush: .php; .title: .; .notranslate title=""
//emlist{
/**
 * NOVIUS OS - Web OS for digital communication
 *
 * @copyright  2011 Novius
 * @license    GNU Affero General Public License v3 or (at your option) any later version
 *             http://www.gnu.org/licenses/agpl-3.0.html
 * @link http://www.novius-os.org
 */

namespace Nos;

class Controller_Admin_Login extends Controller
{
…略…
    protected function post_login()
    {
        if (\Nos\Auth::login($_POST['email'], $_POST['password'], (bool) \Input::post('remember_me', false))) {
            if (\Event::has_events('user_login')) {
                \Log::deprecated('Event "user_login" is deprecated, use "admin.loginSuccess" instead.', @<raw>{|latex|\n}'Chiba.2');
                \Event::trigger('user_login');
            }
            \Event::trigger('admin.loginSuccess');
            return true;
        }
        \Event::trigger('admin.loginFail');
        return __('These details won’t get you in. Are you sure you’ve typed the correct email address and @<raw>{|latex|\n}password? Please try again.');
    }
}
//}

== どういうときに役に立つのか

アプリケーションを全て自分で作っている場合は、わざわざイベントトリガーを用意するメリットはあまりないかもしれません。トリガーを使わずに書いても良いでしょう。トリガーが威力を発揮するのは、パッケージやアプリケーションをオープンソースで公開する場合だと思います。

Novius OSのログイン失敗時のログを取りたい場合、上述の@<tt>{post_login}メソッドを直接書き換えるというやり方でも、ログ機能を追加することはできます。でもこの方法だと、元のアプリケーション（Novius OS）のアップデート時に書き直しすることになります。

では、イベントを活用した場合はどうでしょう。Novius OSではトリガーが用意されています。カスタマイズしたい人は、トリガーにフックするコードを自作し、@<tt>{bootstrap.php}に記述します。この方法だと、アプリケーションのコードに手を入れることなく、処理を追加することができます。自分で追加した部分は元のアプリケーションのコードから分離でき、アップデートが楽になります。

もちろん、イベントを使ってカスタマイズするには、トリガーが設定されていなければなりません。なので、パッケージやアプリケーションを公開して、他の人にも使ってもらいたい、という場合には、イベントトリガーを設定しておくと良いでしょう。

== まとめ

イベントを使うことで、フレームワークやアプリケーションのコードに手を入れることなく、処理を追加することができます。オープンソースで公開するアプリケーションには、イベントのトリガーを用意しておくと、カスタマイズしやすくなります。

//quote{
@<strong>{@ounziw}

Novius OS のコア貢献者であり、日本語対訳ファイルの作成も行っている。

Twitter: @<href>{https://twitter.com/ounziw,@ounziw}

Blog: @<href>{http://ounziw.com/,http://ounziw.com/}
//}
