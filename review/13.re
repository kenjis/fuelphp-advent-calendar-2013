
= FuelPHP（TwitterBootstrap3）でJQueryのプラグインのdataTablesを使う


@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}の13日目です。  


 昨日は@<href>{https://twitter.com/madmamor,@madmamor}さんの  


 @<href>{http://madroom-project.blogspot.jp/2013/12/fac20131212.html,FuelPHPでChatWorkパッケージを使ってみる}  


 でした。@<br>{}
 私もチャットワークのAPIを利用できるようになったのでこの週末に早速試してみたいと思います。  


 それでは今日のお題ですがdataTablesです。@<br>{}
 Webアプリを作るとき、業務系の管理画面を作るとき、FuelPHPの軽量さと作りやすさは強力です。@<br>{}
 私もここ最近のWebアプリケーションはFuelPHPで作ることが大半ですね。@<br>{}
 その中で特に管理画面を作るときはTwitterBootstrapは非常に便利です。@<br>{}
 皆様も日頃お世話になっていますよね？@<br>{}
 また今日ご紹介するdataTablesも強力なJqueryのプラグインです。@<br>{}
 dataTablesはhtmlのテーブルタグを読み込み、ソートや検索を始め、多くの機能を提供します。@<br>{}
 この2つを組み合わせることで表出力が簡単で便利になるのでご紹介します。  


 @<href>{http://datatables.net/,dataTables}  


 @<href>{http://baalzephon.no-ip.org/tech/index.php?JavaScript%2FjQuery%2FDataTables,日本語まとめwiki}  


@<href>{http://datatables.net/release-datatables/extras/TableTools/bootstrap.html,TwitterBootstrapでdataTablesを使う}  


 ただ、TwitterBootstrapもFuelPHP1.7で2から3に変わりました。@<br>{}
 TwitterBootstrap3は2と互換性のない変更が多く、過去の資産が使いまわせません。  


 ※TwitterBootstrap3用の書籍で非常にわかりやすかったのでオススメです  


 DataTablesも例に漏れず上記の方法では無理です。@<br>{}
 TwitterBootstrap3のUIに合わせるのには公式サイト以外の方法が必要です。@<br>{}
 とは言ってもすでに3用のプラグインが作成されています。@<br>{}
 そこでdataTablesの簡単な使い方とTwitterBootstrap3のUIに合わせる方法をご紹介します。  


 まずは最新のソース・ファイルをFuelPHPに配置します。  


 @<href>{https://github.com/DataTables,GirHub　dataTables}  


 本体@<br>{}
 @<href>{https://github.com/DataTables/DataTables/blob/master/media/js/jquery.dataTables.js,https://github.com/DataTables/DataTables/blob/master/media/js/jquery.dataTables.js}  


 Bootstrap用CSS@<br>{}
 @<href>{https://github.com/DataTables/Plugins/blob/master/integration/bootstrap/3/dataTables.bootstrap.css,https://github.com/DataTables/Plugins/blob/master/integration/bootstrap/3/dataTables.bootstrap.css}  


 Bootstrap用JS@<br>{}
 @<href>{https://github.com/DataTables/Plugins/blob/master/integration/bootstrap/3/dataTables.bootstrap.js,https://github.com/DataTables/Plugins/blob/master/integration/bootstrap/3/dataTables.bootstrap.js}  


 Bootstrap用image@<br>{}
 @<href>{https://github.com/DataTables/Plugins/tree/master/integration/bootstrap/images,https://github.com/DataTables/Plugins/tree/master/integration/bootstrap/images}  


 これらをpublic/asset/のそれぞれに配置します。@<br>{}
 ここで注意があります。@<br>{}
 FuelPHPの画像を置く場所はデフォルトはimgです。@<br>{}
 ですがCSSのファイルパスはimagesになっています。@<br>{}
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


 とすれば自動的に読み込んで整形してくれます。@<br>{}
 これでよく要望に上がるソート・ページャー・サーチを一度に対応することが出来ます。@<br>{}
 更によりBootstrapっぽくするには  

#@# lang: .brush:javascript name="code"
//emlist{
var Table = $('#table_id').dataTable({
    //日本語対応したい場合はGithubのpluginから該当のファイルをダウンロードしてください
    oLanguage: {
        sUrl: "/assets/js/plugins/i18n/Japanese.lang"
    },
    //Table生成後に実行されます
    fnInitComplete: function() {
        $('#table_id').each(function() {
            var datatable = $(this);
            //サーチのinputタグをよりBootstrapUIよりに
            var search_input = datatable
                    .closest('.dataTables_wrapper')
                    .find('div[id$=_filter] input');
            search_input.attr('placeholder', 'Search')
                    .addClass('form-control input-sm');
            //表示列数の表示をよりBootstrapUIよりに
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
 他にもdataTablesには多様な追加機能がpluginとして用意されています。@<br>{}
 公式サイトは情報が古かったりするのでGitHubのサンプルのindex.htmlも参考にしてみてください。  


 さてFuelPHPの良いところ言えばRESTControllerですよね。@<br>{}
 またdataTablesはデータをJSONで受け取ることが出来ます。@<br>{}
 大きいdataでもAjaxでJSONを受けることができるので肥大したTableでも対応することが出来ます。@<br>{}
 今日は13日の金曜日ですしJSONを使った例もご紹介します。  


 簡単な例ですと  

#@# lang: .brush:javascript name="code"
//emlist{
var Table = $('#table_id').dataTable({
    //読み込み中の表示
    bProcessing: true,
    //dataを全て読み込む前に表示を始める
    bDeferRender: true,
    //読み込むJSONのURL（RESTControllerのURL）
    sAjaxSource: "/api/dataTables.json",
    //配列名（デフォルトはaadata）
    sAjaxDataProp: "setJson",
    //表示列とjsonの項目をマッピングします
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
            //サーチのinputタグをよりBootstrapUIよりに
            var search_input = datatable
                    .closest('.dataTables_wrapper')
                    .find('div[id$=_filter] input');
            search_input.attr('placeholder', 'Search')
                    .addClass('form-control input-sm');
            //表示列数の表示をよりBootstrapUIよりに
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


 としてRESTControllerは  

#@# lang: .brush:javascript name="code"
//emlist{
public function action_dataTables() {
        $aadata['setJson'][] = ['id'=>1, 'name'=>'hage'];
        $aadata['setJson'][] = ['id'=>1, 'name'=>'fuga'];
        return $this->response($aadata);    
    }
//}


といった感じでいつもどおり配列を渡してJSONを生成するだけです。@<br>{}
 htmlは次の通り  

#@# lang: .brush:html name="code"
//emlist{


            id
            name




//}


とするとtbodyの内容をJSONを元に自動生成してくれます。@<br>{}
 もちろん作りこみ次第で例の最初に全て読み込むのではなく、APIとViewが連携して適時やりとりすることが出来ます。@<br>{}
 このAPI側が非常に簡単につくれるのはFuelPHPのメリットです。  


 以上のようにFuelPHPとdataTablesの相性は抜群です。@<br>{}
 是非、公私でアプリ作成時に試して見てください！  


 それで引き続き@<href>{http://atnd.org/events/45096,FuelPHP Advent Calendar 2013}をお楽しみください！！
