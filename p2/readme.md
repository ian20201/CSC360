1. **Structures and Global Variables**:
   - I have defined a structure named `Vehicle` which holds the attributes of a train such as its unique ID, direction, priority, loading time, crossing time, and the time when it gets set.
   - There's another structure `Node` which will be used for creating a linked list to manage the trains that are ready to be scheduled.
   - Several global variables like `head`, `tail`, `mtx`, `cv`, `last_route`, `same_route_count`, `init_time`, and `next_vehicle_id` have been defined to manage the state of the application.
2. **Time Calculation**:
   - The function `elapsed_time` calculates the time elapsed since the beginning of the program using the `gettimeofday` function.
3. **Queue Management**:
   - I've implemented various functions (`add_to_queue`, `remove_from_queue`, `queue_length`, `peek_queue`, `sort_queue`) for managing the linked list which behaves as a queue. This queue holds trains ready to be scheduled. Which is different from my original though the List is not repersent station anymore
4. **Vehicle Thread**:
   - The function `vehicle_thread` is the main logic where each train (as a thread) waits for its turn to get scheduled based on its priority and other conditions.
   - When a train is ready (after its loading time), it is added to the queue.
   - If the conditions are met (like the direction of previous trains and priority), the train crosses the main track.
5. **Main Function**:
   - In the main function, I first check the command line arguments, read the input file, and populate the `vehicles` array.
   - For each train in the `vehicles` array, I create a thread. The function `vehicle_thread` acts as the entry point for each thread.
   - Finally, I wait for all the threads to complete using `pthread_join`.