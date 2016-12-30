DESCRIPTION
       sem_wait()  decrements (locks) the semaphore pointed to by sem.  If the
       semaphore's value is greater than zero, then  the  decrement  proceeds,
       and  the function returns, immediately.  If the semaphore currently has
       the value zero, then the call blocks until either it  becomes  possible
       to  perform the decrement (i.e., the semaphore value rises above zero),
       or a signal handler interrupts the call.

       // der-NOTE: I'm almost certain now that this function works like the classical sem. definition, i.e., the sem. value never goes negative because the test is done before the decrement. If you find the sem. i.e.
       // my recall test: classical sem. def.=bank account with no overdrafts allowed. If your account is == 0 when you call wait(), they bank will hold you hostage in the building until someone else(yo moma perhaps) deposits $1 into your account. This bank only deals in single integer dollars. i.e. +-$1 transactions only.
       wait(S) {
        while(S <= 0); // spin.
        S--; // dec.
       }

       signal(S) {
        S++; // inc.
       }
       /// my recall test : sem. with wait/sleep q. Its like a bank where they charge you -$1 as soon as you enter the bank. A restricted overdraft is allowed- i.e. If after you pay the $1 entrance fee your account have $0 or MORE,(S > 0), you may proceed onwards with ur personal business. If your balance is negative after paying the entrance fee then you are drugged to sleep until someone deposits $1 into the account and wakes you up to free you. IF your account is negative at any point, then the abs. value of the balance== the number of sleeping hostages in the bank.

struct {
int n;
process_list *list;
} Sem;

wait(Sem *s) {
s->n--;

if(s->n < 0) {
    insert prcs P into s->list;
    sleep(P).
}

}

signal(Sem *s) {
    s->n++;
    if(S <= 0) {
        remove next prcs P from s->list;
        wakeup(P);
    }
}

       sem_trywait() is the same as sem_wait(), except that if  the  decrement
       cannot  be immediately performed, then call returns an error (errno set
       to EAGAIN) instead of blocking.

