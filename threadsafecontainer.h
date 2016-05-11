#include <atomic>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <condition_variable>
// Non Blocking algorithm 
// https://www.research.ibm.com/people/m/michael/podc-1996.pdf
// folly (Facebook (company)'s C++ library)

template<class T>
class ThreadSafeContainer{
public:
	//typedef T value_type;

	// make the queue non-copyable
	//queue_lock(const queue_lock& ) = delete;
	//queue_lock& operator = (const queue_lock& ) = delete;

	explicit ThreadSafeContainer(int size) 
		: size_(size),
		queue_(static_cast<T*>(std::malloc(sizeof(T) * size))),
		read_idx_ (0), 
		write_idx_ (0)
	{
		shutdown_ = false;
		assert( size >= 2);
		if (!queue_)
			throw std::bad_alloc();
	}

	// only one thread runs the destructor
	// 
	~ThreadSafeContainer(){
		if (!std::is_trivially_destructible<T>::value){
			size_t read = read_idx_;
			size_t end = write_idx_;
			while (read!=end){
				//std::cout << queue_[read];
				queue_[read].~T();
				if (++read == size_)
					read = 0;
			}
		}

		std::free(queue_);
	}


  void add(T value) {		
		if (shutdown_) throw std::runtime_error("shutdown exception");		

		//no concurrent add/remove
		std::unique_lock<std::mutex> lk(m_write_);
		unsigned int current;
		unsigned int next;
		
		cv_full_.wait(lk, [&] {
			current = write_idx_.load(std::memory_order_relaxed);
			next = current + 1;
			if (next == size_) {
			  next = 0;
			}
			if (shutdown_) std::runtime_error("shutdown exception");
	   		return (next != read_idx_.load(std::memory_order_acquire));
	    		// wait: queue is full
	   		}); 
		new (&queue_[current]) T(value);
	    write_idx_.store(next, std::memory_order_release);
		//std::cout << "adding:" << current <<endl ;
		lk.unlock();
	 	cv_empty_.notify_one();	  
 	}

	void remove() {
		if (shutdown_) throw std::runtime_error("shutdown_exception");
		std::unique_lock<std::mutex> lk(m_write_);	
		unsigned int current;
	    //unsigned int const current = read_idx_.load(std::memory_order_relaxed);
	    cv_empty_.wait(lk,[&]{
			current = read_idx_.load(std::memory_order_relaxed);
			if (shutdown_) throw std::runtime_error("shutdown_exception");
			return (current != write_idx_.load(std::memory_order_acquire));
			// wait: queue is empty
		});

	    unsigned int next = current + 1;
	    if (next == size_) {
	      next = 0;
	    }
	    queue_[current].~T();
	    read_idx_.store(next, std::memory_order_release);
		//std::cout << "removing:" << current <<endl;
	 	lk.unlock();
    		cv_full_.notify_one();	    
	}

	T pop() {
		if (shutdown_) throw std::runtime_error("shutdown_exception");
		std::unique_lock<std::mutex> lk(m_write_);		
	    unsigned int const current = read_idx_.load(std::memory_order_relaxed);
	    cv_empty_.wait(lk,[&]{
			if (shutdown_) throw std::runtime_error("shutdown_exception");
	    	return (current != write_idx_.load(std::memory_order_acquire));
		});

	    unsigned int next = current + 1;
	    if (next == size_) {
	      next = 0;
	    }
		T value = queue_[current];

		//std::cout << "removing:" << queue_[current] <<endl;
	    queue_[current].~T();
	    read_idx_.store(next, std::memory_order_release);
	 	lk.unlock();
	  	cv_full_.notify_one();	    
		return value;
	}

	void clear(){
		//block everything else and clear the queue
		std::unique_lock<std::mutex> lk(m_read_);	
		std::unique_lock<std::mutex> lk2(m_write_);	

		size_t read = read_idx_;
		size_t end = write_idx_;
		while (read!=end){
			queue_[read].~T();
			if (++read == size_)
				read = 0;
		}

		read_idx_.store(0, std::memory_order_release);
		write_idx_.store(0, std::memory_order_release);

		lk.unlock();
		lk2.unlock();
	}

	bool empty(){
		// is queue empty
		size_t read = read_idx_;
		size_t end = write_idx_;
		return (read==end);
	}

	void shutdown(){
		//activate the shutdown exception
		shutdown_=true;
		cv_empty_.notify_all();
		cv_full_.notify_all();
	}


	private:
	volatile bool shutdown_;
  	const int size_;
  	T* const queue_;
  	std::atomic<unsigned int> read_idx_;
  	std::atomic<unsigned int> write_idx_;  	
   	std::mutex m_write_;
	std::condition_variable cv_full_;
  	std::mutex m_read_;
	std::condition_variable cv_empty_;	 	
};
