= FuelPHPとFluentdを連携させてみる


@<href>{http://atnd.org/events/45096,FuelPHP AdventCalendar 2013}の20日目です。
今回はFluentd＋FuelPHPについて書きたいと思います。@<br>{}


まず、「Fluentdって何？」と思った方は@<href>{http://fluentd.org/,http://fluentd.org/}を参照してください。
今回の目標はFluentdとObserverを連携して、データを送れるようにします。@<br>{}


手順は次のとおりです。

 1. ライブラリの取得
 1. packagesの準備
 1. Observerの設定


== 1. ライブラリの取得


Fluentd用のPHPのLoggerは@<href>{https://github.com/fluent/fluent-logger-php,https://github.com/fluent/fluent-logger-php}にあります。


これをfuel/packages/fluentd/vendor/以下に入れます。

== 2. packagesの準備


fuel/packages/fluentd/以下にbootstrap.phpを作成します。




//emlist[fuel/packages/fluentd/bootstrap.php]{
<?php
require_once __DIR__.'/vendor/Fluent/Autoloader.php';

Autoloader::add_classes(array(
    'Fluentd\\Observer_Td' => __DIR__.'/classes/observer/td.php',
));
//}


このAutoloaderに合わせてfuel/packages/fluentd/classes/に必要なファイルを記述します。




//emlist[fuel/packages/fluentd/classes/observer/td.php]{
<?php
class Observer_Td extends \Orm\Observer {
    public function after_save(\Orm\Model $obj) {
       $save_data = array();
       foreach (array_keys($obj->properties()) as $p) {
           $save_data[$p] = $obj->{$p};
       }

       $instance = new \Fluent\Logger\FluentLogger('localhost', '24800', array(), null);
       $instance->post('tag_name_for_fluentd', $save_data); 
    }
} 
//}

== 3. Observerの設定


Observerを設定して実際にmodelのデータをFluentdに渡します。

//emlist{
<?php
class Model_Model extends \Orm\Model {
    protected static $_observers = array(
        'Fluentd\Observer_Td' => array(
            'events' => array('after_save')
        )
    );
}
//}


これでmodelでsaveメソッドが実行された時にsaveされたデータをFluentdに渡すことが出来ます。


これと同様(特に1、2の部分)にしてFuelPHPのデバッグログやエラーログをFluentdに渡すことも出来ます。


//quote{
@<strong>{Altsencturely}

東京のPHP大好きおじさん 最近はFluentdを使ってログ収集 + 分析基盤ごにょごにょ

Twitter: @<href>{https://twitter.com/Altsencturely,@Altsencturely}

Blog: @<href>{http://takashi-kun.hatenablog.com/,http://takashi-kun.hatenablog.com/}
//}
