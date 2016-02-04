# Resource Usage Library

This is a statically linkable version of the GNU Time program.  Much of the
code is ported from that project.  You can find more informaiton about the
GNU Time project at [[http://directory.fsf.org/wiki/Time]].

# Usage

This library can be used in two ways.  It can be used to track resources used
by the entire process in much the same way as GNU Time.  It can also be used to
track resource usage of single threads.

## Process Resources
To track the resources used by a process create a variable of type 
`struct resuse` and pass that to the `resuse_start()` function with the scope
set to `RESUSE_SCOPE_PROC`.  At the point that you want to collect resources at
use the function `resuse_end()` passing in the same `resuse` object.

### Example

~~~C
int main(int argc, char * argv[]){
	struct resuse resuse;
	resuse_start(&resuse, RESUSE_SCOPE_PROC);
	
	// ...
	
	resuse_end(&resuse);
	
	// ...
}
~~~

## Thread Resources
To track the resources used by a single thread follow the same directions as
for a process, but change the scope to `RESUSE_SCOPE_THREAD`.  Note that to
collect thread resources `_GNU_SOURCE` must be defined.

### Example

~~~C
void * thread(void* param){
	struct resuse resuse;
	resuse_start(&resuse, RESUSE_SCOPE_THREAD);
	
	// ...
	
	resuse_end(&resuse);
	
	// ...
}
~~~

## Printing Ouput
After collecting the resource data `libresuse` provides `resuse_fprint()` which
will format and print the results.  `resuse_fprint()` takes as a parameter a
format string which works similarly to `printf()`.  It uses `%` followed by
a unique character that will be expanded into the correct resource information.

### Recognized commands
This is a list of commands that `resuse_fprintf()` recognizes.

  - `%` == A literal `%'
  - `D` == Average unshared data size in KB
  - `E` == Elapsed real (wall clock) time in [hour:]min:sec
  - `F` == major page faults (required physical I/O)
  - `I` == File system inputs
  - `K` == Average total mem usage
  - `M` == Maximum resident set size in KB
  - `O` == File system outputs
  - `P` == Percent of CPU this job got 
  - `R` == Minor page faults
  - `S` == System (kernel) time
  - `U` == User time
  - `W` == Times swapped out
  - `X` == Average amount of shared text in KB
  - `Z` == Page size
  - `c` == Involuntary context switches
  - `e` == Elapsed real time in seconds
  - `k` == Signals delivered
  - `p` == Average unshared stack size in KB
  - `r` == Socket messages received
  - `s` == Socket messages sent
  - `t` == Average resident set size in KB
  - `w` == Voluntary context switches
  
### Example
Printing the real, user, and system time a process existed:

~~~C
resuse_fprint(stdout, "Real Time: %E, User Time: %U, System Time: %S, CPU Usage: %P\n", &resuse);
~~~


