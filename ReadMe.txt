========================================================================
    CONSOLE APPLICATION : ThreadSafeContainer Project Overview
========================================================================

Thread safe is the implementation of a non-blocking thread safe queue. 

/////////////////////////////////////////////////////////////////////////////
The add method will add one element to the FIFO and, if needed, block the caller until space is available.

The remove method will block the caller until an element is available for retrieval.   

The clear method will remove any objects which were added, but not yet removed.

The shutdown method will cause any blocked or future calls to add or remove to throw a ShutdownException

/////////////////////////////////////////////////////////////////////////////



