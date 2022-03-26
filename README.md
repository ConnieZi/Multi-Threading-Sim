# Multi-Threading Sim
## Using monitors to synchronize two different kinds of processes.
We are imaging that there are visitor threads and guide threads that want to enter a museum. <br/>
Under the Covid-19 restrictions:
1. The museum allows no more than 2 guides and 20 visitors inside the museum at the same time.
2. One guide can guide up to 10 visitors at a time. Unless there is no more upcoming visitors, the guides have to wait until admitting as many visitors as they can, which is 10.
3. If the museum is at its max capacity, the newly coming guides or visitors have to wait outside the museum.
4. The visitors can leave after touring without restriction. But, the guides have to wait until all the visitors are left, then they can indicate that they are gonna leave.
5. The guides must leave together. Once a guide finishes admitting, it's job is finished. All it have to do then is waiting to leave as long as there is no visitors inside and no unfinished guide(s).

The restrictions mentioned above are accomplished by using condition variables along with mutex. Mutex is for avoiding race conditions, and the condition variables act as different queues for threads waiting for different reasons (restrictions).
* <img src="https://user-images.githubusercontent.com/55789923/160221877-9508747e-ad54-4191-8c43-1644d59a341a.jpeg" width="500" height="250">


## Code?
* <code>museumsim.c</code> contains the work of how we handling the whole process. <br/> The visitors have 3 functions (behaviors): <code>arrives</code>, <code>tours</code>, <code>leaves</code>. The guides have 4 functions (behaviors): <code>arrives</code>, <code>enters</code>, <code>admits</code>, <code>leaves</code>. Their description is in <code>museumsim.h</code>. <br/>
* In <code>main.c</code>, we can run to simulate the whole process given different number of visitors and guides. The process will be printed on the console.

## Output?
* <code>make debug</code> from current directory to see the results.
* In default, <code>num_visitors</code> is 10 and <code>num_guides</code> is 1.
<img width="279" alt="Screen Shot 2022-03-25 at 10 18 30 PM" src="https://user-images.githubusercontent.com/55789923/160220981-86fc7a03-5120-48cf-83f3-9eecb372e360.png">

* You can set the number of visitors and guides by yourself by typing for example <code>num_visitors=20 num_guides=2 make debug</code>

