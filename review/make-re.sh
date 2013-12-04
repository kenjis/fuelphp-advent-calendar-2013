#!/bin/sh

cd `dirname $0`
cd ..

if ls markdown/*.md > /dev/null 2>&1
  then
    echo "Markdown file exists"
else
  exit;
fi

for file in markdown/*.md; do
  echo $file
  ext=${file##*.}
  #echo $ext
  base=`basename $file .md`
  #echo $base
  md2review "$file" > "review/$base.re"
done
