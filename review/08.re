= FuelPHPを更に使ってみて使えるなと思った拡張ValidationRuleの書き方とCore拡張の小技

タイトル長っw

@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の8日目です。@<br>{}

CMS系のサイト構築は大体どのフレームワークでも問題ないのですが、基幹業務チックな機能要件があると、様々なモデルの組み合わせで入力値の検証をしたりだとか、項目間において依存性のあるチェックだとかが頻発します。

PHPで基幹システム作らない方が・・・というご意見もごもっともで、そのうちもっとナウい言語にシフトしていきたいと思います。

本題に戻るとFuelPHPはその辺が結構柔軟で、やりようによってはいくらでも対応できるので、1年半ぐらい実プロジェクトで使ってきましたが困ることは特になかったです。

強いていえば、ORMでrelatedしてrows_limitするとあばばばってなりますが、クエリビルダとORMの勘所も掴んだので特に問題ありません。

== 1. おさらい

公式ドキュメント（日本語訳）（@<href>{http://fuelphp.jp/docs/1.7/classes/validation/validation.html,http://fuelphp.jp/docs/1.7/classes/validation/validation.html}）

デフォルトの入力検証含め、こちらに書いてある内容でほとんどの入力値の検証は可能です。

== 2. 複数の任意のパラメータを用いて検証したい場合

複数のポストされたパラメータを用いた検証を行いたい場合、拡張Validationを書きますが、
公式サンプルのパラメータの渡し方がなんとも・・・

//emlist[公式のサンプル]{
// app/classes/myrules.php
class MyRules
{
    // note this is a static method
    public static function _validation_unique($val, $options)
    {
        list($table, $field) = explode('.', $options); // デリミタェェ・・・

        $result = DB::select("LOWER (\"$field\")")
            ->where($field, '=', Str::lower($val))
            ->from($table)->execute();

        return ! ($result->count() > 0);
    }

    // note this is a non-static method
    public function _validation_is_upper($val)
    {
        return $val === strtoupper($val);
    }

}

// and call it like:
$val = Validation::forge();

// Note the difference between static and non-static validation rules:

// Add it staticly, will only be able to use static methods
$val->add_callable('MyRules');

// Adds it non-static, will be able to use both static and non-static methods
$val->add_callable(new MyRules());

$val->add('username', 'Your username', array(), array('trim', 'strip_tags', 'required', 'is_upper'))
    ->add_rule('unique', 'users.username');
//}

以下のようにして書くと、わざわざパラメータをデリミタで区切らなくても引数として渡すことができます。

//emlist[一部抜粋＆修正]{
// app/classes/myrules.php
class MyRules
{
    // note this is a static method
    public static function _validation_unique($val, $table, $field) // 引数で渡ってくるよ
    {
        $result = DB::select("LOWER (\"$field\")")
            ->where($field, '=', Str::lower($val))
            ->from($table)->execute();

        return ! ($result->count() > 0);
    }
    …略…
}

…略…
$val->add('username', 'Your username', array(), array('trim', 'strip_tags', 'required', 'is_upper'))
    ->add_rule('unique', 'users', 'username'); // デリミタつけなくても渡せます
//}

== 3. 独自バリデーションはバリデーションクラス作らなきゃダメですか？

答えはノー。再利用性のあるもの(汎用的な独自バリデーション)はクラスにしておいた方が後々使い回しがききますが、例えばログイン検証なんかは再利用しませんよね？

バリデーションクラスを作らずに独自ルールを定義するにはClosureを使います。

//emlist[Closureで書くルール]{
/*
 * Copyright (c) 2013 Yasuyuki Sasaki
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 */

public static function login_validate()
{
    $val = Validation::forge();
    $password = Input::post('password');

    $val->add('email', 'Email')
        ->add_rule('required')
        ->add_rule( // login_validate
            function($email) use ($password) {
                if (! $email || ! $password) {
                    return true;
                }
                $user = Model_User::query()->
                    where('email', $email)->
                    where('password', $password)->
                    where('del_flg', 0)->get_one();

                if ($user) {
                    return true;
                } else {
                    Validation::active()->set_message('closure', 'ログインに失敗しました。');
                    return false;
                }
            });
    $val->add_field('password', 'パスワード', 'required');
    return $val;
}
//}

=== 検証ルールにClosureを使う場合の落とし穴

Closureで複数のルールを定義するとValidation::active()->set_messageで定義したメッセージがおかしなことになります。以下のサンプルを見てください。

わざと検証に失敗する２つのルールをClosureで定義します。

//emlist[ルールをClosureで複数書く]{
$val = Validation::forge();

$val->add('test1', 'test1')->add_rule(
    function($val) {
        Validation::active()->set_message('closure', 'closure message 1');
        return false;
    }
);
$val->add('test2', 'test2')->add_rule(
    function($val) {
        Validation::active()->set_message('closure', 'closure message 2');
        return false;
    }
);
//}

この検証を実行すると以下のようなエラーが出力されます。

//emlist{
closure message 2
closure message 2
//}

後に定義した方のメッセージが先に定義したClosureのメッセージを上書きしてしまうんですね。

よーく公式ドキュメントを読むとそれらしいことが書いてあるのですが、こういう場合はClosureのルールに名前をつけてあげるといいらしいです。(匿名関数に名前つけるってどういうこっちゃと思うかもしれませんが・・・)

//emlist[Closureに名前をつける]{
$val = Validation::forge();

$val->add('test1', 'test1')->add_rule(['closure1' => 
    function($val) {
        Validation::active()->set_message('closure1', 'closure message 1');
        return false;
    }]
);
$val->add('test2', 'test2')->add_rule(['closure2' => 
    function($val) {
        Validation::active()->set_message('closure2', 'closure message 2');
        return false;
    }]
);
//}

見てわかるとおりClosureのルールをキーとClosureの連想配列でadd_ruleに渡してあげます。

こうするとFuelPHPの中で「このClosureのルールで検証に失敗したらこのメッセージを表示する」というような関連づけが行われるようになります。

== 4. Core拡張の小技

ついでの小ネタ。

拡張バリデーションを定義する際、デフォルトのルールにないけど汎用的な検証を行うというケースがあると思います。

//emlist[sample]{
$val = Validation::forge();
$val->add_callable(new Validate_Common()); // 汎用的な独自ルール
//}

数カ所から呼ばれるならこれでもよいかもしれません。
ですが、これが数十とかになったりすると汎用ルールのadd_callable書くの面倒くさくなりません？
DRY（Don't repeat yourself）しましょ。

//emlist[fuel/app/classes/core/validation.php]{
/*
 * Copyright (c) 2013 Yasuyuki Sasaki
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 */

class Validation extends \Fuel\Core\Validation
{
    protected function __construct($fieldset)
    {
        parent::__construct($fieldset);
        $this->add_callable(new Validate_Common());
    }
}
//}

//emlist[fuel/app/bootstrap.phpのオートローダ部分]{
Autoloader::add_classes(array(
    'Validation'          => __DIR__.'/classes/core/validation.php',
    'View'                => __DIR__.'/classes/core/view.php',
));
//}

これでadd_callableしなくてもバリデーション定義に汎用ルールが使えるようになります。@<br>{}
同じ要領で汎用的なViewの表示処理やデータ整形などもViewクラスを拡張するとDRYできるようになります。

//emlist[fuel/app/core/view.php]{
class View extends \Fuel\Core\View
{
    public function hogeFormat($val)
    {
        // なんか整形する処理
    }
}
//}

そんなこんなで「もうバリデーションなんて怖くないぞ」という感じにまとまりました。

== 最後に

コミュニティも盛り上がり、各地で実績が増え、弊社でもFuelPHPが定着化してきました。

私自身PHP歴は２年弱、FuelPHPは１年半とPHPを初めて間もなくFuelPHPと出会いました。

元々他言語においてもフレームワークを追っかけるのが好きだったので、PHPでもZend、Symfony、CakePHP、CodeIgniter、Kohana、どれにしようかな神様の言う通りせっ(ry 状態だったのですが、FuelPHP選んでよかったです(笑)

//quote{
@<strong>{@sa2yasu}

趣味がストリートダンスのロールキャベツ系男子ぺちぱー。
青森県青森市、宮城県石巻市を中心に何かしら活動してます。

Twitter: @<href>{https://twitter.com/sa2yasu,@sa2yasu}

Blog: @<href>{http://qiita.com/ya-sasaki@github/items/e238f86cabce3acfbe53,http://qiita.com/ya-sasaki@github/items/e238f86cabce3acfbe53}
//}
