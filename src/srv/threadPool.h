#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

/**
 * @brief class ThreadPool responsible for easy thread creation and management
 */
class ThreadPool {
public:
   /**
    * @brief     constructor, copy ctor and assigment operator are deleted
    * @param[in] thrdCount the number of concurrent threads in pool, it is limited with machin's cores count
    * @param[in] runInfinite flag telling that thread's routine should run in infinite loop, or just once
    */
   ThreadPool(unsigned thrdCount = 1,
                bool runInfinite = true):
      _running(false),
      _pausing(false),
      _paused(0),
      _runInInfiniteLoop(runInfinite)
   {
      if(0 == thrdCount) {
         thrdCount = 1;
      } else if(std::thread::hardware_concurrency() < thrdCount) {
         thrdCount = std::thread::hardware_concurrency();
      }
      _thrds.resize(thrdCount);
   }

   /// @destructor - just make it virtual
   virtual ~ThreadPool() { }

   /// @brief creates the threads and holds them in internal collector
   bool Start()
   {
      if(_running) {
         return false;
      }
      _pausing = false;
      _running = true;
      for(unsigned i=0; i<_thrds.size(); ++i) {
         _thrds[i].reset(new std::thread(RunThrd, this));
      }
      return true;
   }

   /// @brief sets the exit flag and waits for all threads in pool to end
   void Stop(bool force = true)
   {
      _running = false;
      if(force) {
         for(unsigned i=0; i<_thrds.size(); ++i) {
            std::unique_ptr<std::thread>& t = _thrds[i];
            if(t) {
               _thrds[i]->interrupt();
            }
         }
      }
      WaitForEnd();
      OnStop();
   }

   /// @brief waits for all threads in pool to end
   void WaitForEnd()
   {
      for(unsigned i=0; i<_thrds.size(); ++i) {
         std::unique_ptr<std::thread>& t = _thrds[i];
         if(t) {
            t->join();
            t.reset();
         }
      }
   }

   /// @brief temporarily pauses thread pool
   void Pause()
   {
      _pausing = true;
      while(_thrds.size() != _paused) {
         std::this_thread::sleep_for(time::Duration::from_millis(50));
      }
   }

   /// @brief resumes the paused thread pool
   void Resume()
   {
      _pausing = false;
      while(0 != _paused) {
         std::this_thread::sleep_for(time::Duration::from_millis(50));
      }
   }

   /// @brief returns true if thread pool is in working state
   bool IsRunning() const
   {
      return _running;
   }

protected:
   /// @brief pure virtual function, extended classes should implement their work here.
   ///        this function calls in infinite loop while thread pool is in runnning state
   virtual void RunInLoop() = 0;

   /// @brief extended classes can implenet this function to do some jobs after stoping the threads
   virtual void OnStop() {}

private:
   void CheckForPause()
   {
      if(!_pausing) {
         return;
      }
      {
         std::lock_guard<std::mutex> lock(_mutex);
         ++_paused;
      }
      while(_pausing) {
         std::this_thread::sleep_for(time::Duration::from_millis(50));
      }
      {
         std::lock_guard<std::mutex> lock(_mutex);
         --_paused;
      }
   }

   static void RunThrd(ThreadPool* pThis)
   {
      if(!pThis) {
         return;
      }
      try {
         pThis->CheckForPause();
         pThis->RunInLoop();
         if(!pThis->_runInInfiniteLoop) {
            pThis->_running = false;
         }
         while(pThis->_running){
            pThis->CheckForPause();
            pThis->RunInLoop();
         }
      } catch (...) {
	      std::cerr << "UNNOWN EXCEPTION in ThreadPool::RunThrd" << std::endl;
         throw;
      }
   }

private:
   volatile bool                              _running;
   volatile bool                              _pausing;
   volatile unsigned                          _paused;
   const bool                                 _runInInfiniteLoop;
   mutable std::mutex                         _mutex;
   std::vector<std::unique_ptr<std::thread> > _thrds;

private:
   ThreadPool(const ThreadPool&);
   ThreadPool& operator=(const ThreadPool&);
};

#endif //_THREADPOOL_H_
