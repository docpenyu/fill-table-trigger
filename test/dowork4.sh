numnhit=10000000
max=10000000000

row=18000


for begin in 2106000 3159000 4738500 7107750 10661625 15992438 23988656 35982984 53974477 80961715 121442572 182163858 273245788 409868681 614803022 922204533 1383306800 1922204533 2922204533 3922204533 4922204533 5922204533 6922204533 7922204533 8922204533 9922204533
do
    hitrate=`expr $begin / $row`
    ../build/main -r $hitrate -c $row -n $numnhit ;
    echo $begin $hitrate
    sleep 0.2;
done
#for i in {10000..100000000000..10000000000}
#do
    #hitrate=`expr $i / $row`
    ##echo $hitrate $i
    #../build/main -r $hitrate -c $row -n $numnhit ;
    #sleep 0.1;
#done
#hitrate=100000
#for i in {100000..100000000000..10000000000}
#do
    #row=`expr $i / $hitrate`

    #../build/main -r $hitrate -c $row -n $numnhit ;
    #sleep 0.1;
#done
