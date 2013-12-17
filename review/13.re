= FuelPHP（TwitterBootstrap3）でJQueryのプラグインのDataTablesを使う

@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の13日目です。@<br>{}

それでは今日のお題ですがDataTablesです。

Webアプリを作るとき、業務系の管理画面を作るとき、FuelPHPの軽量さと作りやすさは強力です。
私もここ最近のWebアプリケーションはFuelPHPで作ることが大半ですね。
その中で特に管理画面を作るときはTwitter Bootstrapは非常に便利です。
皆様も日頃お世話になっていますよね？

また今日ご紹介するDataTablesも強力なJQueryのプラグインです。
DataTablesはHTMLのテーブルタグを読み込み、ソートや検索を始め、多くの機能を提供します。
この2つを組み合わせることで表出力が簡単で便利になるのでご紹介します。

 * DataTables公式サイト @<href>{http://datatables.net/,http://datatables.net/}
 * @<href>{http://baalzephon.no-ip.org/tech/index.php?JavaScript%2FjQuery%2FDataTables,日本語まとめWiki}
 * @<href>{http://datatables.net/release-datatables/extras/TableTools/bootstrap.html,Twitter BootstrapでDataTablesを使う}

ただ、Twitter BootstrapもFuelPHP 1.7で2から3に変わりました。
Twitter Bootstrap 3は2と互換性のない変更が多く、過去の資産が使いまわせません。@<br>{}

『@<href>{http://www.amazon.co.jp/dp/4777517993,はじめてのBootstrap (I・O BOOKS)}』槙俊明 著、工学社@<br>{}
※Twitter Bootstrap 3用の書籍で非常にわかりやすかったのでオススメです@<br>{}

DataTablesも例に漏れず上記の方法では無理です。
Twitter Bootstrap 3のUIに合わせるのには公式サイト以外の方法が必要です。
とは言ってもすでに3用のプラグインが作成されています。
そこでDataTablesの簡単な使い方とTwitter Bootstrap 3のUIに合わせる方法をご紹介します。@<br>{}

まずは最新のソース・ファイルをFuelPHPに配置します。

 * GitHub DataTables @<href>{https://github.com/DataTables/DataTables,https://github.com/DataTables/DataTables}

本体

 * @<href>{https://github.com/DataTables/DataTables/blob/master/media/js/jquery.dataTables.js,https://github.com/DataTables/DataTables/blob/master/media/js/jquery.dataTables.js}

Bootstrap用CSS

 * @<href>{https://github.com/DataTables/Plugins/blob/master/integration/bootstrap/3/dataTables.bootstrap.css,https://github.com/DataTables/Plugins/blob/master/integration/bootstrap/3/dataTables.bootstrap.css}

Bootstrap用JS

 * @<href>{https://github.com/DataTables/Plugins/blob/master/integration/bootstrap/3/dataTables.bootstrap.js,https://github.com/DataTables/Plugins/blob/master/integration/bootstrap/3/dataTables.bootstrap.js}
Bootstrap用image

 * @<href>{https://github.com/DataTables/Plugins/tree/master/integration/bootstrap/images,https://github.com/DataTables/Plugins/tree/master/integration/bootstrap/images}

これらをpublic/asset/のそれぞれに配置します。@<br>{}

ここで注意があります。
FuelPHPの画像を置く場所はデフォルトはimgです。
ですがCSSのファイルパスはimagesになっています。
これを下記のとおり変更します。

#@# lang: .brush:css name="code"
//emlist{
table.dataTable thead .sorting { background: url('../images/sort_both.png') no-repeat center right; }
table.dataTable thead .sorting_asc { background: url('../images/sort_asc.png') no-repeat center right; }
table.dataTable thead .sorting_desc { background: url('../images/sort_desc.png') no-repeat center right; }

//変更

table.dataTable thead .sorting { background: url('../img/sort_both.png') no-repeat center right; }
table.dataTable thead .sorting_asc { background: url('../img/sort_asc.png') no-repeat center right; }
table.dataTable thead .sorting_desc { background: url('../img/sort_desc.png') no-repeat center right; }
//}

これでasset/imgに配置してもソート用の画像が読み込まれます。

あとはTableにIDを降って

#@# lang: .brush:javascript name="code"
//emlist{
var Table = $('#table_id').dataTable();
//}

とすれば自動的に読み込んで整形してくれます。
これでよく要望に上がるソート・ページャー・サーチを一度に対応することが出来ます。@<br>{}

更によりBootstrapっぽくするには

#@# lang: .brush:javascript name="code"
//emlist{
var Table = $('#table_id').dataTable({
    // 日本語対応したい場合はGitHubのpluginから該当のファイルをダウンロードしてください
    oLanguage: {
        sUrl: "/assets/js/plugins/i18n/Japanese.lang"
    },
    // Table生成後に実行されます
    fnInitComplete: function() {
        $('#table_id').each(function() {
            var datatable = $(this);
            // サーチのinputタグをよりBootstrapUIよりに
            var search_input = datatable
                    .closest('.dataTables_wrapper')
                    .find('div[id$=_filter] input');
            search_input.attr('placeholder', 'Search')
                    .addClass('form-control input-sm');
            // 表示列数の表示をよりBootstrapUIよりに
            var length_sel = datatable
                    .closest('.dataTables_wrapper')
                    .find('div[id$=_length] select');
            length_sel.addClass('form-control input-sm')
                    .change(function() {
                        oFC.fnRedrawLayout();
                    });
            oFC.fnRedrawLayout();
        });
    }
});
//}

とするといい感じになります。@<br>{}

他にもDataTablesには多様な追加機能がpluginとして用意されています。
公式サイトは情報が古かったりするのでGitHubのサンプルのindex.htmlも参考にしてみてください。@<br>{}

さてFuelPHPの良いところ言えばREST Controllerですよね。@<br>{}

またDataTablesはデータをJSONで受け取ることが出来ます。
大きいdataでもAjaxでJSONを受けることができるので肥大したTableでも対応することが出来ます。
今日は13日の金曜日ですしJSONを使った例もご紹介します。

 簡単な例ですと

#@# lang: .brush:javascript name="code"
//emlist{
var Table = $('#table_id').dataTable({
    // 読み込み中の表示
    bProcessing: true,
    // dataを全て読み込む前に表示を始める
    bDeferRender: true,
    // 読み込むJSONのURL（REST ControllerのURL）
    sAjaxSource: "/api/dataTables.json",
    // 配列名（デフォルトはaadata）
    sAjaxDataProp: "setJson",
    // 表示列とjsonの項目をマッピングします
            aoColumns: [
                { mData: "id", sDefaultContent: "" },
                { mData: "name", sDefaultContent: "" },
            ],
    oLanguage: {
        sUrl: "/assets/js/plugins/i18n/Japanese.lang"
    },
    fnInitComplete: function() {
        $('#table_id').each(function() {
            var datatable = $(this);
            // サーチのinputタグをよりBootstrapUIよりに
            var search_input = datatable
                    .closest('.dataTables_wrapper')
                    .find('div[id$=_filter] input');
            search_input.attr('placeholder', 'Search')
                    .addClass('form-control input-sm');
            // 表示列数の表示をよりBootstrapUIよりに
            var length_sel = datatable
                    .closest('.dataTables_wrapper')
                    .find('div[id$=_length] select');
            length_sel.addClass('form-control input-sm')
                    .change(function() {
                        oFC.fnRedrawLayout();
                    });
            oFC.fnRedrawLayout();
        });
    }
});
//}

としてREST Controllerは

#@# lang: .brush:javascript name="code"
//emlist{
public function action_dataTables() {
    $aadata['setJson'][] = ['id'=>1, 'name'=>'hage'];
    $aadata['setJson'][] = ['id'=>1, 'name'=>'fuga'];
    return $this->response($aadata);
}
//}

といった感じでいつもどおり配列を渡してJSONを生成するだけです。@<br>{}

HTMLは次の通り。

#@# lang: .brush:html name="code"
//emlist{
<table id="table_id" class="table table-bordered table-hover table-striped">
    <thead>
        <tr>
            <th>id</th>
            <th>name</th>
        </tr>
    </thead>
    <tbody>
    </tbody>
</table>
//}

とするとtbodyの内容をJSONを元に自動生成してくれます。@<br>{}

もちろん作りこみ次第で例の最初に全て読み込むのではなく、APIとViewが連携して適時やりとりすることが出来ます。
このAPI側が非常に簡単につくれるのはFuelPHPのメリットです。@<br>{}

以上のようにFuelPHPとDataTablesの相性は抜群です。
是非、公私でアプリ作成時に試して見てください！

//quote{
@<strong>{@soudai1025}

@TODO

Twitter: @<href>{https://twitter.com/soudai1025,@soudai1025}

Blog: @<href>{http://soudai1025.blogspot.jp/,http://soudai1025.blogspot.jp/}
//}
