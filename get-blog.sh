#!/bin/sh

if [ $# -eq 0 ]; then
    echo "$0 <URL> <chapter>"
    echo "  eg: $0 http://example.jp/blog/ 01"
    exit
fi

cd `dirname $0`

url="$1"
chap="$2"

php get-blog.php "$url" html "$chap"
git add "html/$chap.html"

php get-blog.php "$url" md "$chap"
git add "markdown/$chap.md"

git commit -m "${chap}日目 HTML&Markdown"

php get-blog.php "$url" adjust "$chap"
php get-blog.php "$url" image "$chap"

if [ -d "markdown/images/$chap" ]; then
    cd review
    ./convert-image.sh "images/$chap"
fi
