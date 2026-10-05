#!/bin/bash
pathToLox="C:\Users\elias\Desktop\prog_språk\Lox_interpreter"

echo "Lox tester, Test individual 1-10, 0 for all tests, -1 for errors"
read -p "select test: " index

testArr[0]="lox_tests/test1.lox"
testArr[1]="lox_tests/test2.lox"
testArr[2]="lox_tests/test3.lox"
testArr[3]="lox_tests/test4.lox"
testArr[4]="lox_tests/test5.lox"
testArr[5]="lox_tests/test6.lox"
testArr[6]="lox_tests/test7.lox"
testArr[7]="lox_tests/test8.lox"
testArr[8]="lox_tests/test9.lox"
testArr[9]="lox_tests/test10.lox"
testArr[10]="lox_tests/error1.lox"
testArr[11]="lox_tests/error2.lox"
testArr[12]="lox_tests/error3.lox"
testArr[13]="lox_tests/error4.lox"

if (($index==0)) 
then
    for i in $(seq 1 10);
    do
        ./lox.exe ${testArr[$i-1]}
    done
else if (($index==-1))
then
	for i in $(seq 11 14);
	do
		./lox.exe ${testArr[$i-1]}
		echo ""
	done
else
    ./lox.exe ${testArr[$index-1]}
fi fi