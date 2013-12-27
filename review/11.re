= FuelPHPをもっとComposerで使う

こんにちは、chatiiです。@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013} 11日目を担当させていただきます。

本日はchatiiがComposerでFuelPHPを入れた話を披露したいと思います。

== Composerを採用したFuelPHPへの不満

さて、みなさんComposerへの理解は進みましたか？chatiiはほぼ日曜プログラマー的レガシー的野良PHPerなわけでして、フレームワークおいしいれす(^q^)→Composerなにそれうまいの？という状態でFuelPHP 1.6を迎えました。『@<href>{http://chatii.net/articles/review/2013/10/php-engineer-book-review.html,PHPエンジニア養成読本}』のおかげで今やそれなりにモダンなPHPerになれたと思います。

さて、Composerのドキュメントを読みますと

//quote{
Composer is a tool for dependency management in PHP.
//}

ComposerはPHPの依存性管理ツールです、と言っております。そこで我らがFuelPHPを見てみますと、そうですね、えらく中途半端な感があります。ちょうどこの頃、こちらのスライドを拝見しまして。

 * @<href>{https://www.slideshare.net/keishihosoba/fuel-phpcomposer,FuelPHPをcomposerに本気で対応させた時の話} from @<href>{http://www.slideshare.net/keishihosoba,Keishi Hosoba}

そうですね、今年のAdvent Calendarにも参加されている@<href>{https://twitter.com/hosopy,@hosopy}さんのスライドですね。

言いたいことは全部言われてしまっていますが、要約すると、「なんで全部Composerで入ってくれないの？」と。利用者の立場から言うとですね、それ以外の立場ってあんまないですけど、「おいらのこの新しいプロジェクトは、FuelPHPに依存してるんですよ！その辺Composer先生にはきっちりやっていただきたい！」

ということで。細羽さんのスライドを参考にしつつ、git submoduleどころか、oilも使わないオレオレなcomposer.jsonを書いてみました。

== 脱git submodule & oil

まず、git submoduleは全てcomposer.jsonに加えることができます。

 * fuel/core
 * fuel/auth
 * fuel/email
 * fuel/oil
 * fuel/orm
 * fuel/parser

うん、でもですね、これだけじゃ動かないんで。fuel/fuelってのがいますよね…。諸々のディレクトリ群と、config.phpなどの設定ファイル。これらを、【必要最低限】用意します。あ、そうそう、oilコマンドも必要なので入手しないと。

何しているかって、ただmkdirしてcurlでダウンロードしてくるだけです。なんともスマートさのかけらも無い。

//emlist{
{
    "name": "chatii/fuelphp_setup",
    "type": "metapackage",
    "description": "FuelPHP Installer (not Oil)",
    "keywords": ["framework"],
    "homepage": "http://chatii.net",
    "license": "MIT",
    "authors": [
        {
            "name": "chatii",
            "email": "contact@chatii.net"
        }
    ],
    "support": {
        "source": "https://github.com/chatii/fuelphp_setup",
        "email": "contact@chatii.net"
    },
    "require": {
        "php": ">=5.3.3",
        "monolog/monolog": "1.5.*",
        "fuelphp/upload": "2.0.1",
        "fuel/core": "1.7",
        "fuel/auth": "1.7",
        "fuel/email": "1.7",
        "fuel/oil": "1.7",
        "fuel/orm": "1.7",
        "fuel/parser": "1.7"
    },
    "repositories": [
        {
            "type": "package",
            "package": {
                "name": "fuel/core",
                "type": "fuel-package",
                "version": "1.7",
                "require": {
                    "composer/installers": "*"
                },
                "source": {
                    "url": "https://github.com/fuel/core.git",
                    "type": "git",
                    "reference": "1.7/master"
                }
            }
        },
        {
            "type": "package",
            "package": {
                "name": "fuel/auth",
                "type": "fuel-package",
                "version": "1.7",
                "require": {
                    "composer/installers": "*"
                },
                "source": {
                    "url": "https://github.com/fuel/auth.git",
                    "type": "git",
                    "reference": "1.7/master"
                }
            }
        },
        {
            "type": "package",
            "package": {
                "name": "fuel/email",
                "type": "fuel-package",
                "version": "1.7",
                "require": {
                    "composer/installers": "*"
                },
                "source": {
                    "url": "https://github.com/fuel/email.git",
                    "type": "git",
                    "reference": "1.7/master"
                }
            }
        },
        {
            "type": "package",
            "package": {
                "name": "fuel/oil",
                "type": "fuel-package",
                "version": "1.7",
                "require": {
                    "composer/installers": "*"
                },
                "source": {
                    "url": "https://github.com/fuel/oil.git",
                    "type": "git",
                    "reference": "1.7/master"
                }
            }
        },
        {
            "type": "package",
            "package": {
                "name": "fuel/orm",
                "type": "fuel-package",
                "version": "1.7",
                "require": {
                    "composer/installers": "*"
                },
                "source": {
                    "url": "https://github.com/fuel/orm.git",
                    "type": "git",
                    "reference": "1.7/master"
                }
            }
        },
        {
            "type": "package",
            "package": {
                "name": "fuel/parser",
                "type": "fuel-package",
                "version": "1.7",
                "require": {
                    "composer/installers": "*"
                },
                "source": {
                    "url": "https://github.com/fuel/parser.git",
                    "type": "git",
                    "reference": "1.7/master"
                }
            }
        }
    ],
    "extra": {
        "installer-paths": {
            "fuel/core/": ["fuel/core"],
            "fuel/packages/auth": ["fuel/auth"],
            "fuel/packages/email": ["fuel/email"],
            "fuel/packages/oil": ["fuel/oil"],
            "fuel/packages/orm": ["fuel/orm"],
            "fuel/packages/parser": ["fuel/parser"]
        }
    },
    "suggest": {
        "mustache/mustache": "Allow Mustache templating with the Parser package",
        "smarty/smarty": "Allow Smarty templating with the Parser package",
        "twig/twig": "Allow Twig templating with the Parser package",
        "mthaml/mthaml": "Allow Haml templating with Twig supports with the Parser package"
    },
    "config": {
        "vendor-dir": "fuel/vendor"
    },
    "scripts": {
        "post-install-cmd": [
            "mkdir -p public",
            "mkdir -p fuel/app/cache fuel/app/logs fuel/app/tmp",
            "mkdir -p fuel/app/classes/controller fuel/app/classes/model fuel/app/classes/view",
            "mkdir -p fuel/app/migrations fuel/app/modules fuel/app/tasks fuel/app/lang",
            "mkdir -p fuel/app/tests fuel/app/vendor fuel/app/views",
            "mkdir -p fuel/app/config/development fuel/app/config/production fuel/app/config/staging fuel/app/c@<raw>{|latex|\n}onfig/test",
            "if [ ! -e public/index.php ]; then curl https://raw.github.com/fuel/fuel/1.7/master/public/index.p@<raw>{|latex|\n}hp -o public/index.php; fi",
            "if [ ! -e public/.htaccess ]; then curl https://raw.github.com/fuel/fuel/1.7/master/public/.htacce@<raw>{|latex|\n}ss -o public/.htaccess; fi",
            "if [ ! -e fuel/app/bootstrap.php ]; then curl https://raw.github.com/fuel/fuel/1.7/master/fuel/app@<raw>{|latex|\n}/bootstrap.php -o fuel/app/bootstrap.php; fi",
            "if [ ! -e fuel/app/config/config.php ]; then curl https://raw.github.com/fuel/fuel/1.7/master/fuel@<raw>{|latex|\n}/app/config/config.php -o fuel/app/config/config.php; fi",
            "if [ ! -e fuel/app/config/db.php ]; then curl https://raw.github.com/fuel/fuel/1.7/master/fuel/app@<raw>{|latex|\n}/config/db.php -o fuel/app/config/db.php; fi",
            "if [ ! -e fuel/app/config/routes.php ]; then curl https://raw.github.com/fuel/fuel/1.7/master/fuel@<raw>{|latex|\n}/app/config/routes.php -o fuel/app/config/routes.php; fi",
            "if [ ! -e fuel/app/config/development/db.php ]; then curl https://raw.github.com/fuel/fuel/1.7/mas@<raw>{|latex|\n}ter/fuel/app/config/development/db.php -o fuel/app/config/development/db.php; fi",
            "if [ ! -e fuel/app/config/production/db.php ]; then curl https://raw.github.com/fuel/fuel/1.7/mast@<raw>{|latex|\n}er/fuel/app/config/production/db.php -o fuel/app/config/production/db.php; fi",
            "if [ ! -e fuel/app/config/staging/db.php ]; then curl https://raw.github.com/fuel/fuel/1.7/master/@<raw>{|latex|\n}fuel/app/config/staging/db.php -o fuel/app/config/staging/db.php; fi",
            "if [ ! -e fuel/app/config/test/db.php ]; then curl https://raw.github.com/fuel/fuel/1.7/master/fue@<raw>{|latex|\n}l/app/config/test/db.php -o fuel/app/config/test/db.php; fi",
            "curl https://raw.github.com/fuel/fuel/1.7/master/oil -o oil",
            "php oil r install"
        ]
    },
    "minimum-stability": "dev"
}
//}

//noindent
ださいですね。

使い方は簡単。簡単にREADMEを書いています。

@<href>{https://github.com/chatii/fuelphp_setup,https://github.com/chatii/fuelphp_setup}

 * @<tt>{git clone git@github.com:chatii/fuelphp_setup.git}して
 * @<tt>{cd}で移って
 * @<tt>{rm -rf ./.git/}でchatii/fuelphp_setupの情報消して
 * @<tt>{git init}して
 * @<tt>{curl -sS https://getcomposer.org/installer | php}でComposer.phar手に入れて
 * @<tt>{php composer.phar install}でFuelPHPをインストール

=== 解消したかった手間

 * oilでインストールすると、FuelPHPのリポジトリがめんどくさい
 * coreも含めてバージョン管理したい
 * Vagrantと連携させてさくっと開発に着手したい

お、Vagrant？と思われた方、実はVagrantfileとchef-soloのレシピを用意しました。

 * @<href>{https://github.com/chatii/fuelphp_vagrant,chatii/fuelphp_vagrant}

こちらも簡単ですがREADMEを用意していますので参照してください。Vagrantについての説明は省きますが、簡単に使い方を。

このVagrantfileのあるディレクトリに、chatii/fuelphp_setupクローンしてあげてください。クローン時に名付けたディレクトリ名を、Vagrantfileのsrc_dirに指定してあげます。また、ステージング環境を指定する場合はfuel_envに指定してください。デフォルトではdevelopmentを指定しています。

あまりVagrant、Chefについても詳しくありませんが、個人的にはFuelPHPの開発開始までの手間がかなり削減されました。

無駄なWelcomeも無いため、そのままoil generateを使ってモデルなりコントローラーなり作ってもいいですし、自分で手打ちしてもいいですし。

== 終わりに

FuelPHPも 1.x系が現行の1.7で開発は終わり、ということで、今回用意したcomposer.jsonもどうせ今だけしか使えない OR 使わないんじゃないかなぁ、と思ってます。が、自分が作った物が誰かの目に触れる、役に立つことになれば、それはとてもとても嬉しいので公開しました。

FuelPHP 2.0ではComposerへの対応ってどの程度なんでしょうか？@<fn>{fuel2}情報を追っていないため、今回作ったものはすぐに用無しになるかもしれませんが、そのほうがいいと思ってます。

//footnote[fuel2][［編注］FuelPHP 2.0からはすべてがComposerからインストールできるようになる予定です。ただし、2.0の最初のリリース目標は2014年春とされており、現在はまだComposer経由でインストールできるようにはなっていません。]

最後に。@hosopyさん、スライド勝手に拝借しました。この場を借りて、お礼申し上げます。ありがとうございました！

//quote{
@<strong>{chatii}



Twitter: @<href>{https://twitter.com/chatii0079,@chatii0079}

Blog: @<href>{http://chatii.net/,http://chatii.net/}
//}
