#!/bin/bash

rm -f openomf.bk
bktool -n -o openomf.bk
bktool -f openomf.bk -o openomf.bk -k palette --push
bktool -f openomf.bk -o openomf.bk -k palette -k 0 --import openomf.gpl
bktool -f openomf.bk -o openomf.bk -k background --import openomf.png

