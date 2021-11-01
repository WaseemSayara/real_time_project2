
gcc -w orchestrator.c -o orchestrator
gcc -w hall.c -o hall
gcc -w bus.c -o bus
gcc -w officer.c -o officer
gcc -w passenger.c -o passenger

echo "compilation finished"
while [ 1 -gt 0 ]
do
   echo "Do you want to start the program [y/n]"
   
   read var1

   if [ "$var1"  == "y" -o "$var1" == "Y" ];
    then
    ./orchestrator
    exit 0
    elif [ "$var1" == "n" -o "$var1" == "N" ];
    then
    exit 1
    fi
done
