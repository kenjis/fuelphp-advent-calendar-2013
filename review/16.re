= FuelPHPのmoduleを使いこなす

こんにちは、@<href>{https://twitter.com/hosopy,hosopy}です。

@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013} 16日目を担当させていただきます。

本日は、「FuelPHPのmoduleを使いこなす」のお話をしたいと思います。

== FuelPHPのmoduleのおさらい

FuelPHPのmoduleについては、ここでは割愛させて頂きます。
「moduleとは何だっけ？」な方は、下記のサイトでざっくりとおさらいしておきましょう。

 * @<href>{http://fuelphp.com/docs/general/modules.html,http://fuelphp.com/docs/general/modules.html}
 * @<href>{http://d.hatena.ne.jp/dix3/20111212/1323660316,http://d.hatena.ne.jp/dix3/20111212/1323660316}

== テーマ

それでは本題。
「使いこなす」と風呂敷を広げてしまいましたが、内容としては「moduleの階層化」になります。

まず、以下のようなFuelPHPのモジュール構成を考えます。

//emlist{
[root]
└── fuel
    └── app
        └── modules
            ├── admin
            ├── api
            └── user
//}

このようなモジュール構成をとり始めると、
以下のようなURLの構成を実現したくなったりします。

 * @<tt>{/admin/controller/action/params}
 ** adminモジュールの機能が動作する

 * @<tt>{/api/controller/action/params}
 ** apiモジュールの機能が動作する

 * @<tt>{/user/controller/action/params}
 ** userモジュールの機能が動作する

 * @<tt>{/admin/user/controller/action/params}
 ** userモジュールの機能が、adminモジュールのコンテキストで動作する

 * @<tt>{/api/user/controller/action/params}
 ** userモジュールの機能が、apiモジュールのコンテキストで動作する

つまり、admin、apiという基本モジュールの中に、userモジュールをアドオンしていくというイメージです。

「いや、自分は@<tt>{/user/admin}になってもいいわ」という方はここで終了ですｗ

== サンプルコード

GitHubにサンプルコードを置きました。

@<href>{https://github.com/hosopy/fuel_module_sample,https://github.com/hosopy/fuel_module_sample}

@<tt>{README.md}の再掲になりますが、
開発サーバを動作させてURLを叩くと、以下の様な挙動を示すと思います。

 * @<tt>{/admin}
 ** adminモジュールの@<tt>{root/index}アクションが呼ばれます
 ** これは、@<tt>{fuel/app/modules/admin/config/routes.php}の@<tt>{_root_}定義によるものです

 * @<tt>{/admin/user}
 ** userモジュールの@<tt>{admin/index}アクションが呼ばれます。

 * @<tt>{/admin/user/analytics}
 ** userモジュールの@<tt>{admin/analytics/index}アクションが呼ばれます。

 * @<tt>{/api}
 ** apiモジュールの@<tt>{root/index}アクションが呼ばれます
 ** これは、@<tt>{fuel/app/modules/api/config/routes.php}の@<tt>{_root_}定義によるものです

 * @<tt>{/api/user}
 ** userモジュールの@<tt>{api/index}アクションが呼ばれます。

 * @<tt>{/api/user/analytics}
 ** userモジュールの@<tt>{api/analytics/index}アクションが呼ばれます。

 * @<tt>{/user}
 ** userモジュールの@<tt>{root/index}アクションが呼ばれます
 ** これは、@<tt>{fuel/app/modules/user/config/routes.php}の@<tt>{_root_}定義によるものです

=== ポイント

@<tt>{fuel/app/modules/admin/classes/controller/admin.php}の処理が全てです。

@<tt>{/admin/**/*}なURLへのリクエストを、まずはController_Adminで受け取り、
適切なモジュールへHMVCでリクエストを転送しています。

//emlist[fuel/app/modules/admin/classes/controller/admin.php]{
<?php
namespace \Admin;

class Controller_Admin extends \Controller_Hybrid
{
    /**
     * /admin/module/controller/action/params へのHTTPリクエストを、
     * /module/admin/controller/action/params へのHMVCリクエストとして転送する
     * 
     * @Override
     */
    public function router($resource, $arguments)
    {
        if (\Module::loaded($resource))
        {
            return \Request::forge($resource.'/admin/'.join('/', $arguments))->execute()->response;
        }
        else
        {
            return parent::router($resource, $arguments);
        }
    }
}
//}

apiモジュールでも、全く同様のことを行っています。

== 何が嬉しいか？

=== コードがスッキリする

モジュール間の連携処理をapp内でゴニョゴニョ実装する手もありますが、
こちらのほうがソースの構成がスッキリすると思います。

== adminモジュールとuserモジュールの疎結合

userモジュールは、adminモジュールの提供する機能を存分に利用してコーディングを行う事ができますが、
adminモジュールからは、userモジュールを前提としたコーディングはしない方がよいでしょう。

この点については、設定ファイルやPHPのinterfaceを活用して、
モジュール間の実装インターフェースの規約が必要になると思います。

実際、自分の仕事では、adminモジュールの管理画面テンプレートへ組み込むViewの受け渡し方法として、
適切なinterfaceを実装したViewModelを利用するなど、実装上の疎結合を保っています。

ただし、やり過ぎると分かりにくくなるのでホドホドに…

== 最後に

ここまでお読み頂き、ありがとうございます。
少しでも皆様のお役に立つことが出来れば幸いです。

//quote{
@<strong>{@hosopy}



Twitter: @<href>{https://twitter.com/hosopy,@hosopy}

Blog: @<href>{http://qiita.com/hosopy/items/0428be74f1c3868c55ba,http://qiita.com/hosopy/items/0428be74f1c3868c55ba}
//}
