There are 4 code files in this project,
to implement TAS, TTAS, CLH and MCS lock respectively.
The code can be compiled and executed using the following commands:-

-- For TAS execution

g++ -std=c++17 TAS-cs21mtech12011.cpp -o tas -lpthread -latomic
./tas

-- For TTAS execution
g++ -std=c++17 TTAS-cs21mtech12011.cpp -o ttas -lpthread -latomic
./ttas

-- For CLH execution
g++ -std=c++17 CLH-cs21mtech12011.cpp -o clh -lpthread -latomic
./clh

-- For MCS execution
g++ -std=c++17 MCS-cs21mtech12011.cpp -o mcs -lpthread -latomic
./mcs



