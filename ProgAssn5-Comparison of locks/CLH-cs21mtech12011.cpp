#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;
using namespace std::chrono;

// creating the lock class
class Lock{
    public:
        virtual void lock(int id) = 0;
        virtual void unlock(int id) = 0;
};

// This class will be used in the algorithm

class Qnode
{
      public:
        atomic<bool> *state = new atomic<bool>(true);
    
        // Constructors 
        Qnode(){}

        Qnode(bool val){
            state->store(val);
        }

};

static thread_local Qnode myNode;
static thread_local Qnode myPred;


class CLH_lock :public Lock 
{
    atomic<Qnode> *tail;
public:
    // creating the constructor
     CLH_lock(){
        tail = new atomic<Qnode>(false);
    }

    // defining the lock function
    void lock(int id){
        
        myNode.state->store(true);
        myPred = tail->exchange(myNode);
        while(myPred.state->load());
    }
    // defining the unlock function
    void unlock(int id){
        
        myNode.state->store(false);
        myNode = myPred;
    }

};


// declaring the global parameters


int n,k;
float lambda1,lambda2;
fstream infile;
CLH_lock *obj;
ofstream of("clh_output.txt");
long long int total_enter_time,total_exit_time;

// Creating the test function
void test(int id)

{   exponential_distribution<double> dis1(lambda1); // creating a random engine generator for sleep
    exponential_distribution<double> dis2(lambda2);
    default_random_engine generator;
    ofstream of("clh_output.txt",ios::app);

    for(int i=0;i<k;i++)
    {   
        auto req_time = system_clock::now();

        of<<i<<"th CS entry request at "<<duration_cast<nanoseconds>(req_time.time_since_epoch()).count()<<" by thread "<<id<<"\n";
        obj->lock(id);
        // calling the lock method of the class

        auto actual_time =system_clock::now();

        // incrementing the total time
        total_enter_time+=duration_cast<nanoseconds>(actual_time - req_time).count();
        sleep(dis1(generator));


        of<<i<<"th CS entery at "<<duration_cast<nanoseconds>(actual_time.time_since_epoch()).count()<<" by thread "<<id<<"\n";

        auto req_exit=system_clock::now();

        of<<i<<"th CS exit request at "<<duration_cast<nanoseconds>(req_exit.time_since_epoch()).count()<<" by thread"<<id<<"\n";
        obj->unlock(id);
        // calling the unlock method of the class

        // exit time 
        auto exit_time=system_clock::now();
        total_exit_time+=duration_cast<nanoseconds>(exit_time - req_exit).count(); 

        // updating file for exit
        of<<i<<"th CS exit at "<<duration_cast<nanoseconds>(req_exit.time_since_epoch()).count()<<" by thread "<<id<<"\n";
        sleep(dis2(generator));
    }

}

// creating the main program
int main()
{   
    // reading the input parameters
    infile.open("inp-params.txt");
    infile>>n>>k>>lambda1>>lambda2;
    obj=new CLH_lock();

    // creating the threads and calling the function
    thread t[n];

    for(int i=0;i<n;i++)
        t[i]=thread(test,i);

    for(int i=0;i<n;i++)
        t[i].join();

    // closing the file
    of.close();

    // printing the stats

    cout<<"The number of threads used are: "<<n<<endl;
    cout<<"The average time taken to enter the CS is :"<<(double)total_enter_time/(n*k)<<" ns"<<endl;
    cout<<"The average time taken to exit the CS is :"<<(double)total_exit_time/(n*k)<<" ns"<<endl;


}