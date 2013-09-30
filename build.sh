rm -f a.out
#g++ -fpermissive -g3 -DDEBUG -D_DEBUG -g -rdynamic *.cpp -lpthread 2>&1
g++ -fpermissive -O *.cpp -lpthread 2>&1
