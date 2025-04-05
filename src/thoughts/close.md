---
title: Hacking closures into C is actually kinda easy
date: April 4, 2025
---

## Language functionality

I don't believe there that many times I've thought: "I wish this language had _more_ features!".
This is mainly because all higher-level languages I'm familiar with (I'm including C in this) have sufficient functionality for development.
Meaning, I can (mostly) express and solve any problem in the _best way_ I know how regardless of the language.

That is, of course, not always true.

## The desire for closures in C

To motivate why closures are desirable for systems programming, consider the case of implementing threading on a lightweight OS kernel.
When tasks get preempted, the core they run on recieves an interrupt from the interrupt controller, which changes the execution context into kernel mode and the processor begins running instructions from the timer interrupt handler.

One way to implement this naively might look like:

```C
// Called whenever we recieve a timer interrupt.
//
// NOTE: `ctx` is the old execution context.
void timer_interrupt_handler(void *ctx) {
    // XXX: Interrupts are disabled here.
    // If we allowed them, they could accumulate and overflow the stack!
    //
    // TODO: Things (e.g., keep track of time)

    // Switch directly to another task, if we are available to do so.
    if (my_task.can_context_switch()) yield();
}
```

This is one of the main motivators of having a hardware timer.
If there is a task that hangs or hogs CPU resources, forcing it to switch ensures that other tasks get a chance to run.

But, an issue can be raised at this point.
To yield, a thread must fetch another task off of the data structure used by the task-scheduling algorithm.
It's completely conceivable that this data structure (colloquially known as the ready queue) is shared between all available cores.
In this case, to ensure synchronization, the ready queue needs to enforce mutual exclusion.
This means that the yielding thread may theoretically try (and fail) to access the resouce forever.
Since we have interrupts disabled, this means we can't handle any other kind of interrupt on the core running the yielding thread.

This smells bad.
The details of how this degrades are system- and implementation-dependent, but the lack of guarantee here is troubling.

Of course, this is the use for [first-level and second-level interrupt handlers](https://en.wikipedia.org/wiki/Interrupt_handler#Divided_handlers_in_modern_operating_systems).
We can have the first-level handler switch to a kernel thread running the second-level handler (getting off of the stack with interrupts disabled) and _then_ do intensive tasks like fighting for a mutex and fetching from the ready queue.

Let's recap what we have so far:

```C
void switch_to_kernel_thread(task_t from, func_t cleanup) {
    // Actually performs the context switch, to another misc kernel thread
    // and runs `cleanup`.
}

void second_level_timer_interrupt_handler(task_t from) {
    // 1. Get ready thread
    // 2. Context switch
    // 3. Profit
}

void timer_interrupt_handler(void *ctx) {
    // Switch directly to the second-level handler, if we can.
    if (my_task.can_context_switch())
        switch_to_kernel_thread(
            my_task,
            second_level_timer_interrupt_handler
        );
}
```

This is pretty ergonomic!
But, there's a problem.
This code won't actually work, as nice as it is.

Notice how our cleanup function doesn't have any context...

```C
void switch_to_kernel_thread(task_t from, func_t cleanup) {
    // Stuff...

    // This is how we want it called in _this_ case,
    // but it's certainly not general.
    cleanup(from);
}
```

How can we encode the information that the `cleanup` function needs to run?
This is a prime usecase of closures.
We could capture everything we could want to know when we run cleanup at the time we create it.

## The hack

I called this a hack, but that's clickbait.
It is true that closures should be a feature implemented by the compiler, but it's also possible to implement it in C itself.
How?

Well, let's start by considering what a closure is, exactly.
It's a function with some captured elements from the surrounding scope.
That's not too hard to encode...

```C
// This notation can be confusing at times.
//
// We've named a type `func_t` which is a function pointer to a
// function that takes in `void` and returns `void`.
typedef void (*func_t)(void);

typedef struct closure_t {
    func_t clo_fn;              // The function to capture
    size_t clo_captures_n;      // The number of captured variables we have
    capture_t *clo_captures;    // The actual captured values
} closure_t;
```

Now all that's left is to figure the `capture_t` datatype.
We could always start with something like this:

```C
typedef struct capture_t {
    capture_tn cap_typename; // Enum of the types in below union
    union {
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        void *v;
        char *c;
    } cap_thing;
} capture_t;
```

We are somewhat limited in the types that we can represent, but that's easily extensible.

Creating a closure in this way (as a user) is quite simple (if not a bit more clunky than in a language like C++).
But how do we actually use it?
It turns out that our approach is pretty straightforward: we run `clo_fn` with all of the captures in `clo_captures` as arguments.
Then, it'll appear to the user as though the `clo_captures` were actually captured, even if it's impossible to do so.

But `clo_fn` is a `func_t`, which (according to our `typedef`) takes no arguments.
We could cast it, like so:

```C
typedef void (func_w_args_t*)(/* ... */);
((func_w_args_t)clo_fn)(/* ... */);
```

But such a thing would need to be constructed dynamically, which I am not sure is possible even with the use of the C preprocessor.

This seems like a dead-end until we figure out we've been thinking too hard about getting C to do the work for us.
This is the pretty beautiful thing about the language, in my opinion.
It translates pretty directly to assembly, but in the case that we can't do exactly what we want with the language, we can just _write assembly inline_.

```C
// We consider an implementation targeting Linux for x86-64.
// Simply because this is the kind of system I built `close` on.
void call_closure(closure_t c) {
    // ...
    __asm__ volatile("mov %0, %%rdi;" // Move the first capture into `%rdi`
                    :
                    : "r"(c.clo_captures[0].cap_thing)
                    : "%rdi");
    c.clo_fn();
}
```

This is where some of you scoff and throw trash at me and stuff.
_Ow_.
But, I think it's a pretty natural solution to the problem.
It does, however, come with the downside of portability.
Since each architecture has it's own calling convention, we need to specify the implementation for _each_ architecure we want to target.

There's also the issue that this code (as written) is not actually guaranteed to work.
Why?
Because there is no guarantee that `%rdi` is untouched between our manual modification and the function call.
While this sounds like a non-issue in the case of a single register, when you consider that closures can have enough captures to fill up all 6 argument registers, there becomes a nonzero chance of a fuddle.
Of course, this is easy to check by looking at the `objdump` of the `close` binary to ensure that those registers remain untouched, but it's still sketchy.

I haven't actually gotten this to fail in practice, but it should be simple enough to change.
Ideally, the `c.clo_fn()` should also be a `call` in the asm, but at this point the whole function is just an assembly wrapper.

## Use?

Overall this project turned out a little less elegant than I would like.
The API isn't perfect, and it's not totally bug-free.
However, it was a super interesting problem to think about.
And, who knows, I may revisit it someday if I ever implement threading in pure C.
