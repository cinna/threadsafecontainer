// ThreadSafeContainer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "threadsafecontainer.h"
#include "gtest/gtest.h"


using namespace std;
ThreadSafeContainer<int> queue(10);
const int iterations = 50;
const int producer_thread_count = 16;
const int consumer_thread_count = 16;

void producer()
{
    for (int i = 0; i <= iterations; ++i) {
		try{
        queue.add(i);
		}
		catch(std::exception e){
		std::cout << e.what();
		}
    }
}
void consumer()
{
    for (int i = 0; i <= iterations; ++i) {
        queue.remove();
    }
}

TEST(queue, add_remove)
{
    ThreadSafeContainer<int> f(64);

    f.add(1);
    f.add(2);

    int i1(0), i2(0);

	i1 = f.pop();
    EXPECT_EQ(i1, 1);

	i2 = f.pop();
    EXPECT_EQ(i2, 2);

}

TEST(queue, is_empty)
{
    ThreadSafeContainer<int> f(64);
    EXPECT_EQ(f.empty(),true);

	f.add(1);
	f.add(2);

    int i1(0), i2(0);

	i1 = f.pop();
    EXPECT_EQ(i1, 1);

	i2 = f.pop();
    EXPECT_EQ(i2, 2);

    EXPECT_EQ(f.empty(), true);
}

TEST(queue, clear_the_queue)
{
		ThreadSafeContainer<int> f(64);
	    for (int i = 0; i < 63; ++i) {
			try{
			f.add(i);
			}
			catch(std::exception e){
			std::cout << e.what();
			}
		}
		f.clear();
		EXPECT_EQ(f.empty(), true);
		

}



TEST(queue, thread_safe_add_remove)
{
	std::thread my_threads[producer_thread_count];
	std::thread my_threads1[consumer_thread_count];
	try{
		for (int i = 0; i != producer_thread_count; ++i)
			my_threads[i]=std::thread(producer);
		for (int i = 0; i != consumer_thread_count; ++i)
			my_threads1[i]=std::thread(consumer);  
   
	}
	catch(std::exception e){
		std::cout << e.what();
	}
	for (int i = 0; i != producer_thread_count; ++i)
		my_threads[i].join();
	for (int i = 0; i != consumer_thread_count; ++i)
		my_threads1[i].join();
	
	EXPECT_EQ(queue.empty(), true);
}

TEST(queue, shutdown)
{
	queue.shutdown();
	try
	{
	    queue.add(0);
	}
	catch( const std::exception& err )
	{
		ASSERT_STREQ( "shutdown exception", err.what() );
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	::testing::InitGoogleTest(&argc, argv);	
	return RUN_ALL_TESTS();	
}

