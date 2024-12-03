while ((begin <= end))
do
    hitrate=`expr $begin / $row`
    #echo $begin $hitrate
    ../build/main -r $hitrate -c $row -n $numnhit ;
    ((begin += 50000000))
    sleep 0.2
done