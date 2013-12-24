= FuelPHPがOpAuth対応になったのでfacebookログインをしてみる

@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar2013}参加記事です。@<br>{}


昨年のアドベントカレンダーでは主にPhil Sturgeon氏がメンテされていたNinjAuthというOAuth認証パッケージを使用して簡単にログイン認証が行えるパッケージを作成したのですが、FuelPHP 1.6.1からNinjAuthに代わりOpAuthに対応したパッケージが標準で入るようになったので、今年はそちらでログイン連携を行う最小限の方法を紹介します。ざっくりとした作業量比較としては、インストール作業自体はNinjAuthより楽に行えますが、コントローラ、ビューのサンプルなどは付属してないので、そのあたりはまるっと書く必要があるので一手間ある感じです。@<br>{}

とりあえず動かすところまでなので、バリデーションの処理やコントローラでのハンドリングを実際のアプリケーションで作りこむと良い感じになると思われます。@<br>{}


環境：FuelPHP 1.7.1、Composerはインストール済み、DBはセットアップ済みの想定@<br>{}


それでは各ステップごとにいってみましょう。

== 各configの設定


fuel/app/config/config.phpにalways_loadにauthとormを追加します。

#@# lang: .brush: .php; .title: .; .notranslate title=""
//emlist[fuel/app/config/config.php]{
     'always_load'  => array(

        /**
         * These packages are loaded on Fuel's startup.
         * You can specify them in the following manner:
         *
         * array('auth'); // This will assume the packages are in PKGPATH
         *
         * // Use this format to specify the path to the package explicitly
         * array(
         *     array('auth' => PKGPATH.'auth/')
         * );
         */
         'packages'  => array(
            'auth',
            'orm',
         ),
//}


packages/auth/configからopauth.phpをコピーしてきてapp/config以下に配置し、
opauth.phpのStrategyを追加します。今回はFacebookのみ。

#@# lang: .brush: .php; .title: .; .notranslate title=""
//emlist[fuel/app/config/opauth.php]{
    'Strategy' => [
        'Facebook' => [
            'app_id' => 'xxxxx',
            'app_secret' => 'xxxxxx',
        ],
    ]
//}

== composerで必要なパッケージをインストール


composer.jsonのrequireに以下を追加します。

#@# lang: .brush: .jscript; .title: .; .notranslate title=""
//emlist[composer.json]{
    "opauth/opauth": "0.4.*",
    "opauth/facebook": "dev-master",
//}


プロジェクトのルートディレクトリで

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
composer update
//}

//noindent
を実行すると必要なopauth本体とFacebookストラテジがインストールされます。

== マイグレーションを実行して必要なテーブルを作成

#@# lang: .brush: .bash; .title: .; .notranslate title=""
//cmd{
php oil r migrate --packages=auth
//}

//noindent
を実行すると、下記のテーブルが作成されます。

//emlist{
+------------------------+
| migration              |
| users                  |
| users_clients          |
| users_providers        |
| users_scopes           |
| users_sessions         |
| users_sessionscopes    |
+------------------------+
//}

== コントローラの作成


auth用のコントローラを作成します。コントローラ名に特に縛りはありませんが、今回はController_Authという名前で作成します。@<br>{}

FuelPHPの公式ドキュメントにある@<href>{http://fuelphp.jp/docs/1.7/packages/auth/examples/opauth.html,Using Auth in your application}のサンプルをコピペしつつ、動くように調整してみます。



#@# lang: .brush: .php; .title: .; .notranslate title=""
//emlist[fuel/app/classes/controller/auth.php]{
<?php
/**
 * 認証コントローラーサンプル
 *
 * @author egmc
 * @license  MIT License
 *
 * The Original Code (Part of the FuelPHP Documentation) distributed under the MIT License
 * @link http://fuelphp.jp/docs/1.7/packages/auth/examples/opauth.html
 * @link       http://fuelphp.com
 */
use Fuel\Core\Controller;
use Fuel\Core\Log;

class Controller_Auth extends Controller {

    public function action_oauth($provider = null)
    {
        // bail out if we don't have an OAuth provider to call
        if ($provider === null)
        {
            Log::error(__('login-no-provider-specified'));
            \Response::redirect_back();
        }

        // load Opauth, it will load the provider strategy and redirect to the provider
        \Auth_Opauth::forge();
    }

    public function action_logout()
    {
        // remove the remember-me cookie, we logged-out on purpose
        \Auth::dont_remember_me();

        // logout
        \Auth::logout();

        // and go back to where you came from (or the application
        // homepage if no previous page can be determined)
        \Response::redirect_back();
    }

    public function action_callback()
    {

        // Opauth can throw all kinds of nasty bits, so be prepared
        try
        {
            // get the Opauth object
            $opauth = \Auth_Opauth::forge(false);

            // and process the callback
            $status = $opauth->login_or_register();

            // fetch the provider name from the opauth response so we can display a message
            $provider = $opauth->get('auth.provider', '?');

            // deal with the result of the callback process
            switch ($status)
            {
                // a local user was logged-in, the provider has been linked to this user
                case 'linked':
                    // inform the user the link was succesfully made
                    // and set the redirect url for this status
                    $url = '/';
                    break;

                    // the provider was known and linked, the linked account as logged-in
                case 'logged_in':
                    // inform the user the login using the provider was succesful
                    // and set the redirect url for this status

                    $url = '/';
                    break;

                    // we don't know this provider login, ask the user to create a local account first
                case 'register':
                    // inform the user the login using the provider was succesful, 
                    // but we need a local account to continue
                    // and set the redirect url for this status
                    $url = 'auth/register';
                    break;

                    // we didn't know this provider login, but enough info was returned to auto-register the user
                case 'registered':
                    // inform the user the login using the provider was succesful, and we created a local account
                    // and set the redirect url for this status
                    $url = '/';
                    break;

                default:
                    throw new \FuelException(
                        'Auth_Opauth::login_or_register() has come up with a result '
                        .'that we dont know how to handle.'
                    );
            }

            // redirect to the url set
            \Response::redirect($url);
        }

        // deal with Opauth exceptions
        catch (\OpauthException $e)
        {
            Log::error($e->getMessage());
            \Response::redirect_back();
        }

        // catch a user cancelling the authentication attempt (some providers allow that)
        catch (\OpauthCancelException $e)
        {
            // you should probably do something a bit more clean here...
            exit('It looks like you canceled your authorisation.'
                .\Html::anchor('users/oath/'.$provider, 'Click here')
                .' to try again.');
        }

    }

    public function action_register()
    {

        // create the registration fieldset
        $form = \Fieldset::forge('registerform');

        // add a csrf token to prevent CSRF attacks
        $form->form()->add_csrf();

        // and populate the form with the model properties
        $form->add_model('Model\\Auth_User');

        // add the fullname field, it's a profile property, not a user property
        $form->add_after(
            'fullname', 
            __('login.form.fullname'),
            array(), 
            array(), 
            'username'
        )->add_rule('required');

        // add a password confirmation field
        $form->add_after(
            'confirm', 
            __('login.form.confirm'), 
            array('type' => 'password'), array(), 'password'
        )->add_rule('required');

        // make sure the password is required
        $form->field('password')->add_rule('required');

        // and new users are not allowed to select the group they're in (duh!)
        $form->disable('group_id');

        // since it's not on the form, make sure validation doesn't trip on its absence
        $form->field('group_id')->delete_rule('required')->delete_rule('is_numeric');

        // fetch the oauth provider from the session (if present)
        $provider = \Session::get('auth-strategy.authentication.provider', false);

        // if we have provider information, create the login fieldset too
        if ($provider)
        {
            // disable the username, it was passed to us by the Oauth strategy
            $form->field('username')->set_attribute('readonly', true);

            // create an additional login form so we can link providers to existing accounts
            $login = \Fieldset::forge('loginform');
            $login->form()->add_csrf();
            $login->add_model('Model\\Auth_User');

            // we only need username and password
            $login->disable('group_id')->disable('email');

            // since they're not on the form, make sure validation doesn't trip on their absence
            $login->field('group_id')->delete_rule('required')->delete_rule('is_numeric');
            $login->field('email')->delete_rule('required')->delete_rule('valid_email');
        }

        // was the registration form posted?
        if (\Input::method() == 'POST')
        {
            // was the login form posted?
            if ($provider and \Input::post('login'))
            {
                // check the credentials.
                if (\Auth::instance()->login(\Input::param('username'), \Input::param('password')))
                {
                    // get the current logged-in user's id
                    list(, $userid) = \Auth::instance()->get_user_id();

                    // so we can link it to the provider manually
                    $this->link_provider($userid);

                    // logged in, go back where we came from,
                    // or the the user dashboard if we don't know
                    \Response::redirect_back('dashboard');
                }
                else
                {
                    // login failed, show an error message
                    Log::error(__('login.failure'));
                }
            }

            // was the registration form posted?
            elseif (\Input::post('register'))
            {
                // validate the input
                $form->validation()->run();

                // if validated, create the user
                if ( ! $form->validation()->error())
                {
                    try
                    {
                        // call Auth to create this user
                        $created = \Auth::create_user(
                                $form->validated('username'),
                                $form->validated('password'),
                                $form->validated('email'),
                                \Config::get('application.user.default_group', 1),
                                array(
                                        'fullname' => $form->validated('fullname'),
                                )
                        );

                        // if a user was created succesfully
                        if ($created)
                        {
                            // inform the user

                            // link new user
                            $this->link_provider($created);

                            // and go back to the previous page, or show the
                            // application dashboard if we don't have any
                            \Response::redirect_back('/');
                        }
                        else
                        {
                            // oops, creating a new user failed?
                            Log::error(__('login.account-creation-failed'));
                        }
                    }

                    // catch exceptions from the create_user() call
                    catch (\SimpleUserUpdateException $e)
                    {
                        // duplicate email address
                        if ($e->getCode() == 2)
                        {
                            Log::error(__('login.email-already-exists'));
                        }

                        // duplicate username
                        elseif ($e->getCode() == 3)
                        {
                            Log::error(__('login.username-already-exists'));
                        }

                        // this can't happen, but you'll never know...
                        else
                        {
                            Log::error($e->getMessage());
                        }
                    }
                }
            }

            // validation failed, repopulate the form from the posted data
            $form->repopulate();
        }
        else
        {
            // get the auth-strategy data from the session (created by the callback)
            $user_hash = \Session::get('auth-strategy.user', array());

            // populate the registration form with the data from the provider callback
            $form->populate(array(
                    'username' => \Arr::get($user_hash, 'nickname'),
                    'fullname' => \Arr::get($user_hash, 'name'),
                    'email' => \Arr::get($user_hash, 'email'),
            ));
        }
        $form->add('register', '', array('type'=>'hidden', 'value' => '1'));
        $form->add('submit', '', array('type'=>'submit', 'value' => 'submit'));

        // pass the fieldset to the form, and display the new user registration view
        return \View::forge('login/registration')->set('form', $form->build(), false)
                ->set('login', isset($login) ? $login : null, false);
    }

    protected function link_provider($userid)
    {
        // do we have an auth strategy to match?
        if ($authentication = \Session::get('auth-strategy.authentication', array()))
        {
            // don't forget to pass false, we need an object instance, not a strategy call
            $opauth = \Auth_Opauth::forge(false);

            // call Opauth to link the provider login with the local user
            $insert_id = $opauth->link_provider(array(
                    'parent_id' => $userid,
                    'provider' => $authentication['provider'],
                    'uid' => $authentication['uid'],
                    'access_token' => $authentication['access_token'],
                    'secret' => $authentication['secret'],
                    'refresh_token' => $authentication['refresh_token'],
                    'expires' => $authentication['expires'],
                    'created_at' => time(),
            ));
        }
    }

}
//}


action_oauthが起点となるアクションとなり、ここから各サービスへリダイレクトされます。

コールバックはconfigで特に指定していない場合、同コントローラのaction_callbackアクションに帰ってきますので、action_callback内で処理を判定します。

未登録の場合はauth/registerに飛ばしてユーザー登録を行わせ、登録が完了した時点でlink_providerでユーザーの紐付けを行うようになっています。

== ユーザー登録用ビューの作成


今回はfieldsetを使っているので、単純にechoしてとりあえずフォームを出してみましょう。




#@# lang: .brush: .php; .title: .; .notranslate title=""
//emlist[fuel/app/views/login/registration.php]{
<?php
echo $form;
//}

== 確認してみる


コントローラ、ビューまで作成して/auth/oauth/facebookにアクセスするとfacebook認証が行われ、未登録であればregisterに飛ばされます。


//image[96546dc21b175a2f8fe84494c082970c][登録フォーム]{
//}


登録を完了し、再度同じURLからアクセスするとログインとなり、セッションにログイン情報が格納されます。


//image[d9eca634fc0770132699d574a0cf1f94][セッション情報]{
//}


なお、各OAuthプロバイダから取得したトークンなどの情報はusers_providersテーブルに格納されていますので、こちらを利用してfacebookへの登録や、twitterへの投稿などを行う処理を個別に実装出来ます。

//quote{
@<strong>{@egmc}

主にPHPエンジニア。CAMPFIRE、他個人的にももクロアプリなど。

Twitter: @<href>{https://twitter.com/egmc,@egmc}

Blog: @<href>{http://dasalog.eg2mix.com/,http://dasalog.eg2mix.com/}
//}
