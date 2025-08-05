#!/bin/bash

filename=test_cat/test_3_cat.txt
echo ----------------------------------------------*CAT 1
diff <(cat -b "$filename") <(./s21_cat -b "$filename") -s
echo ----------------------------------------------*CAT 2
diff <(cat -e "$filename") <(./s21_cat -e "$filename") -s
echo ----------------------------------------------*CAT 3
diff <(cat -n "$filename") <(./s21_cat -n "$filename") -s
echo ----------------------------------------------*CAT 5
diff <(cat -s "$filename") <(./s21_cat -s "$filename") -s
echo ----------------------------------------------*CAT 6
diff <(cat -t "$filename") <(./s21_cat -t "$filename") -s
echo ----------------------------------------------*CAT 7
diff <(cat "$filename") <(./s21_cat "$filename") -s
