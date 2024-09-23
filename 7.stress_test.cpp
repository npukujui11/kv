#include <iostream>
#include <chrono>   // 用于高精度时间计算
#include <cstdlib>
#include <pthread.h>
#include <time.h>
#include "skiplist.h"

using namespace std;

#define NUM_THREADS 10
#define TEST_COUNT 100000000

SkipList<int, string>* skipList = new SkipList<int, string>(18);  // 创建最大层为18的跳表实例

// 插入元素的线程函数
void *insertElement(void* threadid) {
    intptr_t tid;                                                 // 线程id
    tid = (intptr_t)threadid;                                     // 将void*类型的threadid转换为long类型
    cout << tid << endl;                                          // 输出线程id
    int tmp = TEST_COUNT/NUM_THREADS;                             // 每个线程需要插入的元素个数

    // 循环插入元素
    for (int i=tid * tmp, count = 0; count <  tmp; i++) {
        count++;                                                  // 计数器加1
        skipList->insert_element(rand() % TEST_COUNT, "a");       // 随机生成一个键值对并插入跳表
    }
    pthread_exit(NULL);                                           // 退出线程
    return NULL;                                                  // 返回NULL
}

// 检索元素的线程函数
void* getElement(void* threadid) { 
    intptr_t tid;                                                 // 线程id
    tid = (intptr_t)threadid;                                     // 将void*类型的threadid转换为long类型
    cout << tid << endl;                                          // 输出线程id
    int tmp = TEST_COUNT/NUM_THREADS;                             // 每个线程需要检索的元素个数

    // 循环检索元素
    for (int i = tid * tmp, count = 0; count < tmp; i++) {                  
        count++;                                                  // 计数器加1
        skipList->search_element(rand() % TEST_COUNT);            // 随机生成一个键值并检索跳表
    }
    pthread_exit(NULL);                                           // 退出线程
    return NULL;                                                  // 返回NULL
}

int main() { 
    srand(time(NULL));

    {
        pthread_t threads[NUM_THREADS];                           // 创建线程数组
        int rc;                                                   // 用于存储pthread_create函数的返回值
        int i;                                                    // 循环变量

        auto start = chrono::high_resolution_clock::now();        // 记录开始时间

        // 创建插入元素的线程
        for (i = 0; i < NUM_THREADS; i++) {
            cout << "main() : creating thread, " << i << endl;    // 输出线程创建信息
            rc = pthread_create(&threads[i], NULL, insertElement, (void *)i);  // 创建线程

            if (rc) {                                             // 如果创建线程失败
                cout << "Error:unable to create thread," << rc << endl;  // 输出错误信息
                exit(-1);                                         // 退出程序
            }
        }

        void *ret;                                                // 用于存储线程返回值
        for(i = 0; i < NUM_THREADS; i++) {                        // 循环等待线程结束
            if (pthread_join(threads[i], &ret) != 0) { 
                perror("pthread_create() error");                 // 输出错误信息
                exit(3);                                          // 退出程序
            }
        }

        auto finish = chrono::high_resolution_clock::now();       // 记录结束时间
        chrono::duration<double> elapsed = finish - start;        // 计算时间差
        cout << "Elapsed time: " << elapsed.count() << " s\n";    // 输出时间差
    }

    /*
    {
        pthread_t threads[NUM_THREADS];                           // 创建线程数组
        int rc;                                                   // 用于存储pthread_create函数的返回值
        int i;                                                    // 循环变量

        auto start = chrono::high_resolution_clock::now();        // 记录开始时间

        // 创建插入元素的线程
        for (i = 0; i < NUM_THREADS; i++) {
            cout << "main() : creating thread, " << i << endl;    // 输出线程创建信息
            rc = pthread_create(&threads[i], NULL, getElement, (void *)i);  // 创建线程

            if (rc) {                                             // 如果创建线程失败
                cout << "Error:unable to create thread," << rc << endl;  // 输出错误信息
                exit(-1);                                         // 退出程序
            }
        }

        void *ret;                                                // 用于存储线程返回值
        for(i = 0; i < NUM_THREADS; i++) {                        // 循环等待线程结束
            if (pthread_join(threads[i], &ret) != 0) { 
                perror("pthread_create() error");                 // 输出错误信息
                exit(3);                                          // 退出程序
            }
        }

        auto finish = chrono::high_resolution_clock::now();       // 记录结束时间
        chrono::duration<double> elapsed = finish - start;        // 计算时间差
        cout << "Elapsed time: " << elapsed.count() << " s\n";    // 输出时间差
    }
    */

    pthread_exit(NULL);                                           // 退出线程

    return 0;
}