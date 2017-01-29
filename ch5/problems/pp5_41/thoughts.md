### PLAN:

0) Add code to create threads.
1) Add init. and cleanup functions.
2) Implement barrier().


### THOUGHTS:

* When did I use the pthread cond. var? ANS: The DPP solution.
  * Right, the cond. var was used as a mechanism to wait for their state to be set from HUNGRY to EATING.
  The P. would loop on until the while condition being waited on was TRUE.
  * In this problem, I think we can use the same concept, just a diff. condition for the P to wait on.
  That condition is while(barrier_gate_is_closed) IOW. while(in_barrier_count < NUM_THREADS);
  * From pproj2: How to use a pthread cond var. see below.
    * It seems that the pthread cond. var provides a way for use to give up a lock we have already acquired.
    Why give up the lock after you just acquired it? ANS: The lock is serving 2 purposes here:
    1. M.ex. access to a shared variable, i.e. prevents race conditions from fuking shit up.
    2. Via the cond_wait(mx,cnd_v), a P that just got the lock can lend out the lock it has a acquired .
    Why lend out the lock? Why not do your data update and release the lock? ANS: Because in this scenario the data update should only be done after some event has occured.
    When this event occurs, the "condition" has become true, and the P's can proceed tot he next sequence of instructions.
### Evernote thoughts.

* PP5.41
    * see paper notes.
    * January 27, 2017 first draft solution completed.
    * January 28, 2017 - der-IDEA: what if we had 1 extra thread , just to control the barrier point rules? - Like a guard at the barrier gates. This way, the worker threads need not be involved in the management of the gate opening / closing. // GOOD IDEA- This is thinking outside of the box. // January 28, 2017  decided to implement a different way - but i should try the guard thread method too.
        * Plan:
            *  Add barrier_controller_thread_func()
                * He is started before the worker threads.
                * Worker threads, Instead of waiting for cond_var. count==N, they wait for count == N + 1. This way, all N  worker threads are in the blocked state. when count == N.
                    * Meanwhile, the barrier controller thread waits for cond_var == N, i.e. when all worker threads are at the barrier. When unblocked, he increments count 1 more time, causing the worker threads to be released simultaneously.
                    * Q: How should we reset the count?
                        * Right now, the first thread to re-enter the barrier checks for count==N, then resets it to 0.
                            * Q: Can the guard do this instead?
                            * Q: What if instead of increment to 0 and reset explicitly, we use clock arithmetic, such that the last thread causes an overflow of the count back to 0. and the while condition. becomes false. e.g.
                                * barrier()
                                    * wait(in_sem); // init. to N // acts as a N limit turnstyle
                                    * lock(mx)
                                    * count++; // init to 0,.
                                    * count %= (N + 1);
                                    * cond_signal(cond);
                                    * unlock(mx)
                                    * // wait for gate to open.
                                    * lock(mx)
                                    * while(count != 0)
                                        * cond_wait(cond_var, mx);
                                    * unlock(mx);
                                    * signal(in_sem) // place turnstyle out side this function?
                * PROBLEM with v0 implementation for pprob.5.41
                    * intermittent deadlock where 1 T gets stuck in the entry barrier after all other threads have left. //  is it possible the cond_signal() function works like a sem. where the last thread that gets stuck because he’s waiting on a final signal? //
                        * Debugging - it appears the first guy to enter the barrier gets left behind.
                            * 1 case
                            * Can we make it happen again? Not after 3 runs. Found similar bug.
                                * T0 is stuck at the exit barrier.
                                * Was he the first to enter the exit barrier? YUP.
                                    * what does that imply?
                                        * First- this problem did not show up with only 2 P’s. Hence, in the future, the minimal test should be with 3 Ts/Ps.
                                        * Second/ The first T to enter the gates is always the one that gets left behind. Wether at the entry or the exit barrier. So my guess is this is a detail of cond_variables I misunderstood. IT may also be a quirk of the pthread cond_var def. , after all, we only get the generic definition for cond_variables AND it was in the context of Monitors. TODO: see if the sem+mutex implementation described by galvin explains this behaviour.
                                        * My guess is the T that gets stuck is blocked until a signal is received.
                                        * NOTE: the last T to increment inside_count never calls cond_wait().
                                            * Consequently, There are exactly 3 cond_signal() calls, and only N-1=3-1=2 cond_wait() calls from the first and the second T that enter the entry barrier.
                                            * FIX?
                                            * 0) Make the first T to exit the barrier perform and additional signal on the entry cond_var.-this should unblock the guy getting stuck behind.
                                            * 1) Use a guard thread.
                                            * 2)