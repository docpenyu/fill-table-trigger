end=20000000
for hitrate in 10000 50000
do
    let channel=20
    while ((channel <= end))
    do
        echo $channel $hitrate
        ../build/improved_alg -r $hitrate -c $channel;
        ((channel *= 10))
        sleep 0.2
    done
done