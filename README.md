# ThreadPool for C++

A class whose objects create a bunch of waiting threads ready to have a task pushed on to them. Tasks are functions with arguments bound to them (although arguments are not needed). Thread pools are useful because they have the concurrent advantages of multi-threading without having to deal with overhead due to thread creation and deletion.

## Compiling Your Own threadpool.o

If you want to create your own object file for threadpool.cpp, you will have to temporarily comment out '#include "threadpool_tmpl.cpp"' to stop the definitions from interfering.