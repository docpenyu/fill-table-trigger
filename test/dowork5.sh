numnhit=10000000
max=10000000000

row=18000
total_hitrate=540000000

#for row in 200 400 800 1600 3200 6400 12800 20000 40000 60000 80000 100000 
#do
    #for hitrate in 1000 2000 4000 8000 12000 20000 40000 60000 80000 100000
    #do
        #../build/main -r $hitrate -c $row -n $numnhit ;
        #sleep 0.5;
    #done
#done
begin=10
end=100000
step=40000000
while ((begin <= end))
do
    hitrate=`expr $total_hitrate / $begin`
    echo $begin $hitrate
    ../build/main -r $hitrate -c $begin -n $numnhit ;
    ((begin *= 2))
    sleep 0.2
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
