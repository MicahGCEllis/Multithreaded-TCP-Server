struct FiberContext
{
    void* rsp;

};

extern "C" 
    {
        void switch_context(FiberContext* current, FiberContext* next);
    }