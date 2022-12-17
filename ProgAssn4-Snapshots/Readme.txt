There are two files in the given folder, one for the obstruction free implmentation and other is wait free.

1. For the implementation of each file an input file with all the parameters is required.

2. "inp-params.txt" is also provided in the given folder.

3. For implementing the obstruction free version the following command is required:-

g++ mrmw_obs_cs21mtech12011.cpp -lpthread -latomic -o obs

./obs

It will create an object file which can be executed further.

4. For implementing the wait free version.

g++ mrmw_wait_cs21mtech12011.cpp -lpthread -latomic -o wait

./wait

5. Each program produces an output file to log all the entries of the program.
