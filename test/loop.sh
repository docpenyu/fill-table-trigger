export numnhit=1000000000
export row=18000
export maxH=5000000000
export beg=2109375
export step=100000000

while [ $beg -le $maxH ]
do
    export begin=$beg
    ((beg += $step))
    export end=$beg
    sh test_log.sh > loop_result_231213/test_in_hd101_${beg}.txt &
    sleep 1
done