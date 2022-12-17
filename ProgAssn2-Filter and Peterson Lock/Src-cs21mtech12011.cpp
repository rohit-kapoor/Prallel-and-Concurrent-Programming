#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<mutex>
#include<fstream>
#include<time.h>
#include<math.h>
#include<thread>
#include<vector>
#include<time.h>
#include<unistd.h>
#include<chrono>

using namespace std;

int n,k;
float lambda1,lambda2;  // declaring all the variables as global
long long total_cs_wait,total_exit_wait; // Variables to evaluate the waiting time


fstream infile;
FILE *outfile;

class Lock {
public:
  virtual void lock(int id){};
  virtual void unlock(int id){};
};

class Filter : public Lock
{
public:
	vector <int> level=vector<int>(n,0); // Creates a vector of size n and initializes each value to 0
	vector <int> victim=vector<int>(n);
	

	bool wait(int id,int l)
	{
		if (victim[l]!=id)
			return false;

		for(int i=0;i<n;i++)
		{	
			if (i!=id && level[i]>=l)
				return true;
		}

		return false;
	}

	void lock(int id)
	{	//printf("lock\n");
		for(int i=1; i<n;i++)
		{
			level[id] =i;
			victim[i]=id;

			while(wait(id,i));
		}

	}

	void unlock(int id)
	{	//printf("unlock\n");
		level[id]=0;
	}

};

class peterson : public Lock
{
	public :
	vector <bool> flag=vector<bool>(n,false);
	int victim;

	bool wait(int id)
	{
		if(victim !=id)
			return false;

		for(int i=0; i<n;i++)
		{
			if(i!=id && flag[i])
				return true;
		}
		return false;
	}

	void lock(int id)
	{
		flag[id]=true;
		victim = id;
		while(wait(id));
	}

	void unlock(int id)
	{
		flag[id]= false;
	}
};

class peterson_tree : public Lock 
{
public:
	vector<peterson> tree_lock;

	peterson_tree()
	{
		for(int i=0;i<n;i++)
			tree_lock.push_back(peterson());
	}

	void lock(int id)
	{
		// The locks need to be acquired from the leaf node till the root
		// Now the leaves start in a binary tree from the index n/2 

		int index= n/2 + id/2;

		while(true)
		{
			tree_lock[index].lock(id);
			if(index==0)
				break;
			index=index/2;
		} 

	}

	void unlock(int id)
	{
		int index = n/2 +id/2;
		vector<int> lst;


		while(true)
		{	lst.push_back(index);
			if(index==0)
				break;
			index=index/2;
		}

		for(int i=lst.size()-1;i>=0;i--)
			tree_lock[lst[i]].unlock(id);
	}

};

float getExpRand(float k)
{	//printf("call\n");
	float value=log(1+rand())/k;
	//printf("cal done\n");
	return value;
}


void testCS1(Filter* Test, int id)
{
	// Creating the variables for time


	
	for(int i=1;i<=k;i++)
	{	
		auto enter_request=std::chrono::high_resolution_clock::now();
		fprintf(outfile, "%d th Critical section entry request at %lld by thread %d (msg 1) \n",i,enter_request,id);

		Test->lock(id); 
		// Acquire lock to enter the cs

		auto enter_time=std::chrono::high_resolution_clock::now();

		fprintf(outfile, "%d th Critical section entry at %lld by thread %d (msg 2) \n",i,enter_time,id);

		total_cs_wait+=std::chrono::duration_cast<std::chrono::microseconds>(enter_time - enter_request).count();
		
		sleep(getExpRand(lambda1)); // Sleeping to simulate expensive computational task
		
		auto exit_request=std::chrono::high_resolution_clock::now();

		fprintf(outfile, "%d th Critical section exit request at %lld by thread %d (msg 3) \n",i,exit_request,id);

		Test->unlock(id); //release the lock

		auto exit=std::chrono::high_resolution_clock::now();

		total_exit_wait+=std::chrono::duration_cast<std::chrono::microseconds>(exit - exit_request).count();

		fprintf(outfile, "%d th Critical section exit at %lld by thread %d (msg 4) \n",i,exit,id);

		sleep(getExpRand(lambda2)); //To simulate the remainder section of the program 

	}

}

void testCS2(peterson_tree* Test,int id)
{
	// Creating the variables for time


	
	for(int i=1;i<=k;i++)
	{	auto enter_request=std::chrono::high_resolution_clock::now();
		fprintf(outfile, "%d th Critical section entry request at %lld by thread %d (msg 1) \n",i,enter_request,id);

		Test->lock(id);  
		// Acquire lock to enter the cs

		auto enter_time=std::chrono::high_resolution_clock::now();

		fprintf(outfile, "%d th Critical section entry at %lld by thread %d (msg 2) \n",i,enter_time,id);

		total_cs_wait+=std::chrono::duration_cast<std::chrono::microseconds>(enter_time - enter_request).count();

		sleep(getExpRand(lambda1)); // Sleeping to simulate expensive computational task

		auto exit_request=std::chrono::high_resolution_clock::now();

		fprintf(outfile, "%d th Critical section exit request at %lld by thread %d (msg 3) \n",i,exit_request,id);

		Test->unlock(id); // release the lock

		auto exit=std::chrono::high_resolution_clock::now();

		total_exit_wait+=std::chrono::duration_cast<std::chrono::microseconds>(exit - exit_request).count();

		fprintf(outfile, "%d th Critical section exit at %lld by thread %d (msg 4) \n",i,exit,id);

		sleep(getExpRand(lambda2)); //To simulate the remainder section of the program 

	}

}


void run_peterson()
{
	total_exit_wait=0;
	total_cs_wait=0;

	cout<< "Peterson tree algo stats"<<"\n";

	peterson_tree* Test=new peterson_tree();
	fprintf(outfile, "\n");
	fprintf(outfile, "\n");
	fprintf(outfile, "peterson Tree stats\n");

	std::thread tid[n]; // Creating n threads

	for(int i=0;i<n;i++)
		tid[i]= std::thread(testCS2,Test,i); // Passing the thread id as a parameter to the given function

	for(int i=0;i<n;i++)
		tid[i].join();

	std::cout << "The average waiting time to enter the critial section = "
       << float(total_cs_wait ) / (n * k) << "us\n";
  std::cout << "The average waiting time to exit the critial section = "
       << float(total_exit_wait) / (n * k) << "us\n";

}

void run_filter()
{
	total_exit_wait=0;
	total_cs_wait=0;

	cout<< "Filter algo stats"<<"\n";

	Filter* Test=new Filter();

	fprintf(outfile, "Filter algo stats\n");

	std::thread tid[n]; // Creating n threads

	for(int i=0;i<n;i++)
		{tid[i]= std::thread(testCS1,Test,i); // Passing the thread id as a parameter to the given function
		  //printf("thread call\n");
		}	
	for(int i=0;i<n;i++)
		tid[i].join();

	std::cout << "The average waiting time to enter the critial section = "
       << float(total_cs_wait ) / (n * k) << "us\n";
  std::cout << "The average waiting time to exit the critial section = "
       << float(total_exit_wait ) / (n * k) << "us\n";

}



int main()
{	
	// open the input file and initialize the global params

	infile.open("inp-params.txt");
	infile>>n>>k>>lambda1>>lambda2;
	//printf("%d%d\n",n,k);
	outfile = fopen("output-log.txt","w");

	run_filter(); // Calling the filter algorithm

	run_peterson(); // calling the peterson algorithm

	fclose(outfile);

}