#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<mutex>
#include<fstream>
#include<fstream>
#include<time.h>
#include<math.h>
#include<thread>

using namespace std;

int N,M; // defining two global integers for the range of primes and the no of threads

fstream in_file; // defining a global input file

FILE *dam_file, *sam1_file, *sam2_file,*time_file; // creating global pointers to point to the files



class Counter
{
public:

	long value=1;

	mutex counter_lock;

	long getandincrement()
	{
		counter_lock.lock();
		value++;
		counter_lock.unlock();
		return value;
	}
};


void SAM1();
void SAM2();
void DAM();
bool isPrime(long no);
void static_thread(int k);
void dynamic_thread(Counter *counter);
void static_thread2(int k);




// Main function to call all the functions
int main()
{	
	// opeining the input-param file to read N and M

	in_file.open("inp-params.txt");
	in_file >> N >>M;


	// creating files to store the output

	dam_file= fopen("Primes-DAM.txt","w");
	sam1_file = fopen("Primes-SAM1.txt","w");
	sam2_file = fopen("Primes-SAM2.txt","w");
	time_file = fopen("Times.txt","w");



	// keeping track of the time taken for each function

	time_t start_sam1 = time(NULL);
	SAM1();
	time_t end_sam1 = time(NULL);
	fclose(sam1_file);



	time_t start_dam =time(NULL);
	DAM();
	time_t end_dam =time(NULL);
	fclose(dam_file);


	time_t start_sam2 =time(NULL);
	SAM2();
	time_t end_sam2=time(NULL);
	fclose(sam2_file);


	// printing the times taken by each function
	cout<<"The time taken by sam1 will be : "<<end_sam1-start_sam1<<"\n";
	cout<<"The time taken by DAM will be : "<<end_dam-start_dam<<"\n";
	cout<<"The time taken by sam2 will be : "<<end_sam2 - start_sam2<<"\n";

	// Writing the values to the file times.txt

	
	fprintf(time_file, "%ld ", end_dam-start_dam);
	fprintf(time_file, "%ld ", end_sam1-start_sam1);
	//fprintf(time_file, "Time for SAM2 %d sec\n", end_sam2 - start_sam2);

	fclose(time_file);
	


}


bool isPrime(long no)
{
	if (no==1)
		return false;

	for(long int i=2; i<=sqrt(no);i++)
	{
		if(no%i==0)
			return false;
	}
	return true;
}



void static_thread(int k)
{	
	//cout<<"The thread id is"<<k<<"\n";
	long int j;


	for(j=k;j< pow(10,N);j+=M)  // Allocating 1, m+1,2m+1.. to thread1, 2,m+2,2m+2.. to thread2
	{	
		if (j==0)
			continue;
	
		if(isPrime(j))
		{	//cout<<j<<"\n";
			fprintf(sam1_file,"%ld  ",j); // It will store the given prime number in the file
		}
	}

}

void static_thread2(int k)
{
	long int j;


	for(j=k;2*j+1< pow(10,N);j+=M)  // Allocating 1, m+1,2m+1.. to thread1, 2,m+2,2m+2.. to thread2
	{	
		
	
		if(isPrime(2*j+1))
		{	//cout<<j<<"\n";
			fprintf(sam2_file,"%ld  ",2*j+1); // It will store the given prime number in the file
		}
	}

}


void dynamic_thread(Counter *counter)
{
	long int i=0;
	while(i+1<pow(10,N))
	{
		i=counter->getandincrement();
		//cout<<i<<"\n";

		if(isPrime(i))
			fprintf(dam_file,"%ld  ",i);
	}

}




void SAM1()
{	
	std::thread tid[M]; // Now we can only create M threads
	for(int i=0;i<M;i++)
		tid[i]= std::thread(static_thread,i); // Passing the thread id as a parameter to the given function

	for(int i=0;i<M;i++)
		tid[i].join();
}

void SAM2()
{	
	if(2<pow(10,N))
	{
		fprintf(sam2_file,"%d  ",2); // 2 is the only even prime no
	}

	std::thread tid[M]; // Now we can only create M threads
	for(int i=0;i<M;i++)
		tid[i]= std::thread(static_thread2,i); // Passing the thread id as a parameter to the given function

	for(int i=0;i<M;i++)
		tid[i].join();

}

void DAM()
{	
	Counter count;

	std::thread tid[M]; // Now we can only create M threads
	for(int i=0;i<M;i++)
		tid[i]= std::thread(dynamic_thread,&count); // Passing the thread id as a parameter to the given function

	for(int i=0;i<M;i++)
		tid[i].join();
}