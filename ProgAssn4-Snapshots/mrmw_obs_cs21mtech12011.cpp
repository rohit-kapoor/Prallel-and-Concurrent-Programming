#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<mutex>
#include<fstream>
#include<time.h>
#include<math.h>
#include<thread>
#include<atomic>
#include<unistd.h>
#include<algorithm>
#include<mutex>
#include<bits/stdc++.h>

using namespace std;
using namespace std::chrono;

// creating the clss for obstruction free MRMW 

class snap
{	public:
	long int time_stamp;
	int val;
	snap() noexcept {}

	snap(int v)
	{
		time_stamp=0;
		val=v;


	}

	snap(int t,int v)
	{
		time_stamp=t;
		val=v;
	}
};

class obstruction_free : public snap 
{
private:
	int capacity;
	atomic <snap>  *a_table; // creating an array of atomic MRSW registers
	
public:
	 atomic<snap>* collect() {

	 	atomic<snap>* copy = new atomic<snap>[capacity];

	 	for(int i=0;i<capacity;i++)
	 	{
	 		snap temp(a_table[i].load().time_stamp,a_table[i].load().val);
	 		copy[i].store(temp);

	 	}

	 	return copy;
	 }

	obstruction_free(int capacity,int init)  // creating a constructor and inititalizing it
	{
		 this->capacity = capacity;
            a_table = new atomic<snap>[capacity];  // creating the table of the given capacity


            for (int i = 0; i < capacity; i++) {

                a_table[i]= snap(init);  // not using new keyword might give error
            }
	}

	void update(int thread_id, int l,int value) 
	{
		int me =l;
		snap old_val= a_table[l];
		snap new_val=snap(old_val.time_stamp+1,value);
		a_table[l]=new_val;
	}
	bool check(atomic<snap>* old_copy,atomic<snap>* new_copy )
	{
		for(int i=0;i<capacity;i++)
		{
			if (old_copy[i].load().time_stamp!=new_copy[i].load().time_stamp||old_copy[i].load().val!=new_copy[i].load().val)
				return false;

		}
		return true;
	}

	int* scan() {
		atomic<snap>* old_copy;
		atomic<snap>* new_copy;
		old_copy = collect();

		int *result = new int[capacity];

		while(true)
		{
			new_copy= collect();
			bool flag=true;

			if (check(old_copy,new_copy))
				break;
			else
				old_copy=new_copy;
		}

		for(int i=0;i<capacity;i++)
			result[i]=new_copy[i].load().val;

		return result;

	}

};

int n_w,n_s,M,k;
float u_w,u_s;
fstream in_file;
obstruction_free *obj;   // creating the object for the obstruction free class
ofstream of("obst_free_output.txt");
long long int total_time,max_time;
mutex m;

bool term;

void write_fun(int id)
{
	int v,l;
	srand(time(0)); // setting the seed for random number generation
	//printf("writter called\n");
	exponential_distribution<double> distribution(u_w); // creating a random engine generator for sleep
    default_random_engine generator;


	while(! term)
	{
		// the given code will keep on executing till the term flag is set to false
		v= rand()%100;
		l=rand()%M;
		obj-> update(id,l,v); // updating the location with a random integer

		auto write_time = system_clock::now();
        long long int dur = (duration_cast<nanoseconds>(write_time.time_since_epoch()).count());

        // writing the log into the file
        ofstream of("obst_free_output.txt",ios::app);
       string str;
       str+="Thread "+to_string(id)+" writes a value "+to_string(v)+" at time ";
        of<<str<<dur<<endl;

        usleep(distribution(generator)); // sleeping for exponential time with an average of u_w
        // to simulate complex computation

	}
}

void snapshot(int id)
{
	int j=0;
	 exponential_distribution<double> distribution(u_s);
    default_random_engine generator;  // random engine generator for exponential distribution
    //printf("snapshot thread called\n");
    
    while(j<k)
    {	
    	auto begin = system_clock::now();  // noting the time before the start of the snapshot
    	int *result = obj->scan();
    	// time at completion of the snapshot
    	auto end_time = system_clock::now();
    	long long int end_t=(duration_cast<nanoseconds>(end_time.time_since_epoch()).count());
    	long long int snap_dur = duration_cast<nanoseconds>(end_time - begin).count();

    	// storing the corresponding snapshot in the file
    	string str;
    	ofstream of("obst_free_output.txt",ios::app);

    	for(int i=0;i<M;i++)

    		{
    			str+=" l "+to_string(i+1)+" : "+to_string(result[i]);
    		}
  
    	of<<"Snapshot corresponding to thread "<<id<<endl<<str<<" which finishes at "<<end_t<<endl;
    	m.lock();
    	total_time+=snap_dur;
    	max_time=max(max_time,snap_dur);
    	m.unlock();
    	usleep(distribution(generator));
    	j++;
    }

}

int main()
{
    in_file.open("inp-params.txt");
	in_file >>n_w>>n_s>>M>>u_w>>u_s>>k;
	
	term=false;

	obj = new obstruction_free(M,0); // initializing the obstruction free object with 0 values

	//ofree_file=fopen("obst_free_output.txt","w");
	thread write_thread[n_w];     // creating the writer threads

	thread snapshot_thread[n_s]; // creating the snapshot collector threads

	//fprintf(ofree_file,"Obstruction free snapshot output \n");
	of<<"Obstruction free snapshot output\n";
	for(int i=0;i<n_w;i++)
	{
		write_thread[i] = thread(write_fun,i);   // calling the writer threads
	}

	// calling the snapshot threads

	for(int i=0;i<n_s;i++){
		snapshot_thread[i]=thread(snapshot,i);
		snapshot_thread[i].join();

	}

	term =true; // it will inform all the writer threads that they need to terminate

	for(int i=0;i<n_w;i++)
		write_thread[i].join();

	of.close(); // closing the file

	// Printing worst and average time taken for collecting snapshots
    
    printf("Ratio of writter to snapshot threads is :%lf\n",(double)n_w/n_s);
    printf("Average time for obstruction free snapshot : %lld micro sec\n",total_time/(n_s*k*1000));
    printf("Worstcase time for MRSW snapshot collection: %lld micro sec\n",max_time/1000);

}