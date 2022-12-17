#include <atomic>
#include <thread>
#include <iostream>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<math.h>
#include<vector>
#include<fstream>
#include<random>

using namespace std;

int N,k,lambda;
float p;
long long read_time, write_time;
int no_read,no_write;

ifstream in_file;
FILE *outfile,*outfile2;
std:: atomic<int> shared;  // creating the shared variable which will be read and written upon

float getExpRand()
{	srand(time(0));
	return log(1+rand())/k;
}


class StampedValue 
{	
	public:

	long stamp;
	long value;

	StampedValue() 
	{
	stamp=0;
	value=0;
	 }

	StampedValue(int init) // on calling the constructor stamp is initialized to 0 and value to init
	{
		stamp=0;
		value = init;
	}

	void writeStampedvalue(long s,long v) // The write function will write the time stamp and the value
	{
		stamp =s;
		value =v;
	}

	StampedValue* maxfunc(StampedValue *a,StampedValue *b)
	{
		if (a->stamp>b->stamp)
			return a;
		else
			return b; // since b has the higher id if both of them have the same timestamp return b
	}

};

// defining the register class
class Register
{
	virtual int read(int n)=0; // defining pure virtual functions
	virtual void write(int id,int value, int n)=0; // the class cannot be instantiated
};



class atomicSRSW: public Register
{	public:
	static thread_local long int lastStamp;
	static thread_local StampedValue *lastRead; 
	static StampedValue* rvalue;

	atomicSRSW(int init)
	{
	rvalue = new StampedValue(init);
	lastRead->value=rvalue->value;
	lastRead->stamp=rvalue->stamp;
	lastStamp = 0;
	}

	StampedValue* read()
	{
	StampedValue* value = rvalue;
	StampedValue* last = lastRead;
	StampedValue* result = value->maxfunc(value,last);
	lastRead=result;

	return result;
	}


	void write(long val)
	{
	long int stamp = lastStamp +1;
	rvalue->value=val;
	rvalue->stamp=stamp;
	lastStamp=stamp;

	}
};

class atomicMRSW : public Register
{	
	public:
	static thread_local long int lastStamp;
	StampedValue* a_table1;

	atomicMRSW(int init,int capacity)
	{	
		a_table1= new StampedValue[capacity];
		for(int i=0;i<capacity;i++)
				a_table1[i]=init;

	}

	int read(int id)
	{
		int me = id;
		StampedValue val;
		for(int i=0;i<N;i++)
			val=a_table1[me];
		return id;
	}

	void write(int id,int capacity,int val)
	{
		long stamp = lastStamp+1;
		lastStamp=stamp;
		for(int i=0;i<N;i++)
			a_table1[i]=val;
	}

};


class atomicMRMW : public Register
{
public:
	StampedValue* a_table;  // creating a table of objects pointed by a pointer of object type


atomicMRMW(int capacity,int val)
{
	a_table = new StampedValue[capacity]; // creating the table of the given capacity

	for(int i=0;i<capacity;i++)
		a_table[i].value=val; // It will assign stamp=0 and value=val for each object

}
int read(int capacity)
{
	StampedValue* m = new StampedValue();

	for(int i=0;i<capacity;i++)
		m=m->maxfunc(m,a_table+i);
	return m->value;
}

	void write(int id,int capacity,int value)
{
	int me=id; 
	StampedValue *m = new StampedValue();

	for(int i=0;i<capacity;i++)
		m=m->maxfunc(m,a_table+i);
	a_table[me].stamp=(m->stamp)+1;
	a_table[me].value=value;

}


};

atomicMRMW obj(N,0); // taking the initial value of shared object as 0

void testAtomic(int id)
{
	// we need to compare the performance of the given algorithm with the atomic implementation
	// in c++.
	srand(time(0)); // Initializing the random function with seed as current time

	for(int i=0;i<k;i++)
	{
		// Now each thread can enter into the critical section k times
		
		int flag=0; // it will indicate a read operation

		std::random_device rand;
    	std::mt19937 generator(rand());
    	std::discrete_distribution<> distrib({ p, 1-p });

    	flag=distrib(generator);


		auto request_time=std::chrono::high_resolution_clock::now(); // It will store the request time
		time_t r = std::chrono::system_clock::to_time_t(request_time);
		fprintf(outfile, "%d th action requested at %s by thread %d\n",i,ctime(&r),id );

		if (flag)
		{
			int val=shared; // reading the value associated with the shared variable
			fprintf(outfile, "value read : %d\n", val);
		}
		else
		{
			int val=k*id; // It will be a unique value written by each thread on the shared variable
			shared = val; // writing onto the shared variable
			fprintf(outfile, "Value written : %d\n", val );
		}
		auto completion_time =std::chrono::high_resolution_clock::now(); // it will store the time at which the operation was complete
		time_t c = std::chrono::system_clock::to_time_t(completion_time);
		fprintf(outfile, "%d th action completed at %s by thread %d\n",i,ctime(&c),id );

		if (flag)
			{
				read_time+= std::chrono::duration_cast<std::chrono::nanoseconds>(completion_time-request_time).count();
				no_read+=1;
			}
		else
			{ 
			   	write_time+= std::chrono::duration_cast<std::chrono::nanoseconds>(completion_time-request_time).count();
				no_write+=1;
			}
		sleep(getExpRand()); // sleep for an amount randomly distributed by lambda

	}
}


void testAtomic2(int id)
{
	// we need to compare the performance of the given algorithm with the atomic implementation
	// in c++.
	srand(time(0)); // Initializing the random function with seed as current time

	for(int i=0;i<k;i++)
	{
		// Now each thread can enter into the critical section k times
		
		int flag=0; // it will indicate a read operation

		std::random_device rand;
    	std::mt19937 generator(rand());
    	std::discrete_distribution<> distrib({ p, 1-p });
    	flag=distrib(generator);


		auto request_time=std::chrono::high_resolution_clock::now(); // It will store the request time
		time_t r = std::chrono::system_clock::to_time_t(request_time);
		fprintf(outfile2, "%d th action requested at %s by thread %d\n",i,ctime(&r),id );

		if (flag)
		{
			int val=obj.read(N); // reading the value associated with the shared variable
			fprintf(outfile2, "value read : %d\n", val);
		}
		else
		{
			int val=k*id; // It will be a unique value written by each thread on the shared variable
			obj.write(id,N,val); // writing onto the shared variable
			fprintf(outfile2, "Value written : %d\n", val );
		}
		auto completion_time =std::chrono::high_resolution_clock::now(); // it will store the time at which the operation was complete
		time_t c = std::chrono::system_clock::to_time_t(completion_time);
		fprintf(outfile2, "%d th action completed at %s by thread %d\n",i,ctime(&c),id );

		if (flag)
			{
				read_time+= std::chrono::duration_cast<std::chrono::nanoseconds>(completion_time-request_time).count();
				no_read+=1;
			}
		else
			{ 
			   	write_time+= std::chrono::duration_cast<std::chrono::nanoseconds>(completion_time-request_time).count();
				no_write+=1;
			}
		sleep(getExpRand()); // sleep for an amount randomly distributed by lambda

	}
}


int main()
{
	in_file.open("inp-params.txt");
	in_file>>N>>k>>lambda>>p;
	outfile=fopen("output-log-inbuilt.txt","w");

	//atomicMRMW obj;

	read_time=0;
	write_time=0;
	no_read=0;
	no_write=0;

	std:: thread tid[N]; // It will create N threads 

	for(int i=0;i<N;i++)
		tid[i]=std:: thread(testAtomic,i);  // calling the function from each of the threads
	for(int i=0;i<N;i++)
		tid[i].join();
	cout<< "Stats for in-built atomic register "<<"\n"<<"\n";

	cout<< " The average time taken by read operations is :"<< float(read_time +0)/no_read<<"ns"<<"\n";
	cout<< " The average time taken by write operations is :"<< float(write_time +0)/no_write<<"ns"<<"\n";
	cout<< " the average time taken by both read and write operations is :"<<float(read_time+write_time+0)/N*k<<"ns"<<"\n";
	fclose(outfile);

	//-------- code for running the application-------

	read_time=0;
	write_time=0;
	no_read=0;
	no_write=0;

	outfile2=fopen("output-log-user.txt","w");

	std:: thread tid2[N];


	for(int i=0;i<N;i++)
		tid2[i]=std:: thread(testAtomic2,i);  // calling the function from each of the threads
	for(int i=0;i<N;i++)
		tid2[i].join();

	cout<< "\n"<<"Stats for the MRMW atomic register "<<"\n"<<"\n";

	cout<< " The average time taken by read operations is :"<< float(read_time)/no_read<<"ns"<<"\n";
	cout<< " The average time taken by write operations is :"<< float(write_time)/no_write<<"ns"<<"\n";
	cout<< " the average time taken by both read and write operations is :"<<float(read_time+write_time)/N*k<<"ns"<<"\n";
	fclose(outfile2);

}

