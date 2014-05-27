echo "Running unit tests:"
cd tests

for i in `ls .|grep -E "test_[A-Za-z0-9]"| grep --invert-match -E "*.cpp"`
do
    if test -f $i
    then
        if $VALGRIND ./$i 2>> ./tests.log
        then
            echo $i PASS
        else
            echo "ERROR in test $i: here's tests/tests.log"
            echo "------"
            tail ./tests.log
            exit 1
        fi
    fi
done

echo ""
