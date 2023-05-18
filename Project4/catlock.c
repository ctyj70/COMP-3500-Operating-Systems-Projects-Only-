/*
* Catlock.c
*
* 20 March 2023
*
* Use LOCKS/CV'S to solve the cat syncronization problem in
* the file.
*
*/


#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>

/*
 * Defines the total number of food bowls 
 */

#define numOfBowls 2

/*
 * Total number of times each animal will eat
 */

#define numEat 4 

/*
 * Defines the number of cats
 */

#define numOfCats 6

/*
 * Defines the number of mice
 */

#define numOfMice 2

/*
 * Defines the names of Mice
 */

const char* const mouseNames[numOfAnimalNames] = {"Ichigo", "Saitama", "Luffy", "Levi", "Goku", "Naruto"};

/*
 * Defines the name of cats
 */

const char* const catNames[numOfAnimalNames] = {"Sylvester", "Neko"};


struct lock *bowl_lock[numOfBowls], *eat_lock, *complete_lock; 
int eat = 0; 
int complete = 0;

/*
 * Sets up created locks
 */

static void setup() {
    int i;
    
    for (i = 0; i < numOfBowls; i++) {
        bowl_lock[i] = lock_create("bowl_lock");
        assert(bowl_lock[i] != NULL);
    }
    
    eat_lock = lock_create("eat_lock");
    assert(eat_lock != NULL);
    
    complete_lock = lock_create("complete_lock");
    assert(complete_lock != NULL);
    
    eat = 0;
    complete = 0;
}

static void cleanup() {
    int i;
    
    for (i = 0; i < numOfBowls; i++) {
        lock_destroy(bowl_lock[i]);
    }
    lock_destroy(eat_lock);
    lock_destroy(complete_lock);
}


static void eat(const char *animal, int num, int iteration) {
    int bowl = (int) (random() % numOfBowls);
    lock_acquire(bowl_lock[bowl]);
    kprintf(
        "%s: %s started to eat at bowl %d, iteration %d\n",
        animal,
        (strcmp(animal, "cat") == 0) ? catNames[num] : mouseNames[num],
        bowl + 1, iteration + 1 );
    clocksleep(1);
    kprintf(
        "%s: %s finishes eating at bowl %d, iteration %d\n",
        animal,
        (strcmp(animal, "cat") == 0) ? catNames[num] : mouseNames[num],
        bowl + 1,
        iteration + 1
    );
    lock_release(bowl_lock[bowl]);
}

/*
 * catlock()
 *
 * Arguments:
 * void * unusedpointer: currently unused.
 * unsigned long catnumber -  holds the cat identifier from 0 to numOfCats - 1
 * 
 *
 * Returns - nothing.
 * 
 * Write and comment this function using locks/cv's
 *
 */
static void catlock(void * unusedpointer, unsigned long catnumber) {
    (void) unusedpointer;
    
    int i;
    for (i = 0; i < numEat;) {
        lock_acquire(eat_lock);

   
        if (eat >= 0 && eat < numOfBowls) {

            eat++;
            lock_release(eat_lock);
            eat("cat", catnumber, i);

            lock_acquire(eat_lock);
            eat--;
            lock_release(eat_lock);
            i++;  
            clocksleep(5);  
        } else {
            lock_release(eat_lock);  // Can't eat currently
        }
        thread_yield();
    }
    lock_acquire(complete_lock);
    complete++;
    lock_release(complete_lock);
}


/*
 * mouselock()
 * Arguments:
 * void * unusedpointer: currently unused.
 * unsigned long mousenumber: holds the mouse identifier from 0 to numOfMice - 1.
 * Returns - nothing
 *      Write and comment this function using locks/cv's.
 *
 */
static void mouselock(void * unusedpointer, unsigned long mousenumber) {
    (void) unusedpointer;
    
    int i;
    for (i = 0; i < numEat;) {
        lock_acquire(eat_lock);
       
        if (eat <= 0 && eat * -1 < numOfBowls) {
            eat--;
            lock_release(eat_lock);
            eat("mouse", mousenumber, i);

            lock_acquire(eat_lock);
            eat++;
            lock_release(eat_lock);
            i++;  
            clocksleep(5);  
        } else {
            lock_release(eat_lock);  // Cannot eat at this time
        }
        thread_yield();
    }
    
    lock_acquire(complete_lock);
    complete++;
    lock_release(complete_lock);
}


/*
 * catmouselock()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 * Returns:
 *      0 on success.
 * Notes:
 *      Driver code to start up catlock() and mouselock() threads.  Change
 *      this code as necessary for your solution.
 */
int catmouselock(int nargs, char ** args) {
    int index, error;

    /*
     * Avoid unused variable warnings.
     */

    (void) nargs;
    (void) args;
    setup();

    /*
     * Start numOfCats catlock() threads.
     */

    for (index = 0; index < numOfCats; index++) {

        error = thread_fork("catlock thread", 
        NULL, index, catlock, NULL);

        /*
         * panic() on error.
         */

        if (error) {
            panic("catlock: thread_fork failed: %s\n", strerror(error) );
        }
    }

    /*
     * Start numOfMice mouselock() threads.
     */

    for (index = 0; index < numOfMice; index++) {

        error = thread_fork("mouselock thread",
         NULL, index, mouselock, NULL);

        /*
         * panic() on error.
         */

        if (error) {
            panic("mouselock: thread_fork failed: %s\n", strerror(error));
        }
    }
    
    while (complete < numOfCats + numOfMice) {
        thread_yield();
    }
    
    cleanup();

    return 0;
}