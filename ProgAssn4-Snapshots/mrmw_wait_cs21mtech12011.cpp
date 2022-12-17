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
#include<set>
#include<algorithm>
#include<bits/stdc++.h>

using namespace std;
using namespace std::chrono;

// creating the class for wait free MRMW

class snap
{	
public:
	int pid;
	int val;
	int seq;

	// It is required to for atomic 
	snap() noexcept {}

	snap(int v)
	{
		pid=0;
		val=v;
		seq=0;
	}

	snap(int id,int v,int s)
	{
		pid=id;
		val=v;
		seq=s;

	}
};

class wait_free : public snap
{
private:
	atomic <int*>* Helpsnap;	// creating the helper to store snapshots
	atomic <snap> * reg;
	atomic <int>* sn;
	int capacity;
	int no;

public:
	wait_free(int cap,int init,int n)
	{
		// creating the constructor for the class

		no=n; 
		capacity = cap;
		sn = new atomic<int>[no]; // An atomic array of integers
		reg= new atomic<snap>[capacity]; // creating a register who's size is equal to the capacity
		Helpsnap = new atomic<int*>[no];
		for(int i=0;i<capacity;i++)
		{
			snap temp(init);  // creating a temporary variable to store the values
			reg[i].store(temp); // storing the corresponding value into the register
		}

		for(int j=0;j<no;j++)
			Helpsnap[j] = new int[capacity];	// the helpsnap has been created as an array of an array of integers
		
	}

	// function to update the values in the array of registers

	void update(int t_id,int l,int val)
	{
		sn[t_id]++;
		snap temp(val,t_id,sn[t_id].load());
		reg[l].store(temp);
		Helpsnap[t_id]=scan();
	}

	atomic<snap>* collect()
	{	
		atomic<snap>* copy = new atomic<snap>[capacity];
		for(int i=0;i<capacity;i++)
		{
			snap temp(reg[i].load().val,reg[i].load().pid,reg[i].load().seq);
			copy[i].store(temp);
		}
		return copy;

	}
bool check(atomic<snap>* o,atomic<snap>* n )
	{
		for(int i=0;i<capacity;i++)
		{
			if (o[i].load().pid!=n[i].load().pid || o[i].load().val!=n[i].load().val|| o[i].load().seq!=n[i].load().seq)
				return false;

		}
		return true;
	}

int *scan()
{
		atomic<snap>* o;
		atomic<snap>* n;

		set<int> can_help;
		o=collect();
	while(true)
	{
			n=collect();

		if (check(o,n))
			break;

		for(int i=0;i<capacity;i++)
		{
			if (o[i].load().pid!=n[i].load().pid || o[i].load().val!=n[i].load().val|| o[i].load().seq!=n[i].load().seq)
			{
				if (can_help.find(pid)!=can_help.end())
					{	
					return Helpsnap[n[i].load().pid];
				}
				else{
					can_help.insert(pid);
				
				}
			}

		}
		o=n;
		
	}
	int *result = new int[capacity];
    for (int j = 0; j < capacity; j++)
        result[j] = n[j].load().val;
    return result;
	

}

};

int n_w,n_s,M,k;
float u_w,u_s;
fstream in_file;
wait_free *obj;   // creating the object for the obstruction free class
FILE *wfree_file;
long long int total_time,max_time;
mutex m;
ofstream of("wait_free_output.txt");

bool term;

void write_fun(int id)
{
	int v;
	int l;
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
        ofstream of("wait_free_output.txt",ios::app);
        of<<"Thread: "<<id<<" writes a value "<<v<<" at time "<<dur<<endl;
        usleep(distribution(generator)); // sleeping for exponential time with an average of u_w
        // to simulate complex computation

	}
}

void snapshot(int id)
{
	int j=0;
	 exponential_distribution<double> distribution(u_s);
    default_random_engine generator;  // random engine generator for exponential distribution
    //cout<<"snapshot thread called\n";

    while(j<k)
    {	// noting the time before the start of the snapshot
    	auto begin = system_clock::now();  
    	int *result = obj->scan();
    	auto end=system_clock::now();  // time at completion of the snapshot
    	long long int end_time = (duration_cast<nanoseconds>(end.time_since_epoch()).count());
    	long long int snap_dur = duration_cast<nanoseconds>(end - begin).count();

    	// storing the corresponding snapshot in the file
    	

         string str;
         ofstream of("wait_free_output.txt",ios::app);
  
    	for(int i=0;i<M;i++)
    		{	
    			str += " l" + to_string(i+1)+" : " + to_string(result[i]);
    		}
   
        of<<"snapshot corresponding to thread "<<id<<endl<<str<<" which finishes at "<<end_time<<endl;

        m.lock();
    	total_time+=snap_dur;
    	max_time=max(max_time,snap_dur);
    	m.unlock();

    	usleep(distribution(generator));
    	j++;
    }

}

int main()
{	// reading the input parameters

    in_file.open("inp-params.txt");
	in_file >>n_w>>n_s>>M>>u_w>>u_s>>k;
	term=false;

	// initializing the object
	obj = new wait_free(M,0,n_w+n_s);
	of<<"Wait free snapshot output"<<endl;


	// creating the writer threads and snapshot collector threads


	thread write_thread[n_w];     
	thread snapshot_thread[n_s]; 

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

	// closing the file
	of.close();



	// Printing worst and average time taken for collecting snapshots
    
    printf("Ratio of the no of writer threads to snapshot threads is %lf\n",(double)n_w/n_s);
    printf("Average time for wait free snapshot : %lld micro sec\n",total_time/(n_s*k*1000));
    printf("Worstcase time for MRSW snapshot collection: %lld micro sec\n",max_time/1000);

}