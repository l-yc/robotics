#include <ev3.h>
#include <bits/stdc++.h>
#include <pthread.h> 
using namespace std;

// A normal C function that is executed as a thread
// when its name is specified in pthread_create()
void *myThreadFun(void *vargp)
{
    sleep(1);
    LcdPrintf(0, "Printing GeeksQuiz from Thread \n");
    return NULL;
}

int main() {
    InitEV3();

    //int v[] = {0,1,2,3,4,5,6,7,8,9};
    vector<int> v;
    for (int i = 0; i < 10; ++i) v.push_back(i);
    random_shuffle(v.begin(), v.end());

    setAllSensorMode(COL_REFLECT, COL_REFLECT, COL_REFLECT, COL_REFLECT);
   // while (!ButtonIsDown(BTNEXIT)) {
   //     int val = readSensor(IN_1);
   //     //LcdPrintf(1, "hello %d\n", val);
   //     LcdTextf(1, 0, 0, "hello %d\n", val);

   //     Wait(1000);
   // }

    int d; scanf(" %d", &d);
    LcdPrintf(1, "INTEGER READ %d ", d);
    pthread_t thread_id;
   // printf("Before Thread\n");
    LcdPrintf(1,"Before Thread\n");
    //pthread_create(&thread_id, NULL, myThreadFun, NULL);
    //pthread_join(thread_id, NULL);
   // printf("After Thread\n");
    LcdPrintf(1, "After Thread\n");
   
    for (int i = 0; i < v.size(); ++i) {
        int x = v[i];
        LcdPrintf(x%2, "hello %d\n", x);

        Wait(1000);
    }

    FreeEV3();
    exit(0);
}
