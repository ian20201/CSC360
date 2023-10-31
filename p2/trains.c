//Name : Ian Chen
//V-Number : V00887293
//Date : 2023-10-10
//CSC360 p2
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#define MAX_VEHICLES 100  // Define the maximum number of vehicles.

// Define the structure for a vehicle.
typedef struct {
    int vid;             // Unique ID for the vehicle.
    char route;          // Direction in which the vehicle is heading (either 'E' or 'W').
    int precedence;      // Priority level of the vehicle (either high or low).
    int prep_duration;   // Duration taken by the vehicle to prepare.
    int travel_duration; // Duration taken by the vehicle to travel.
    int set_time;        // Time when the vehicle is ready.
} Vehicle;

// Define the structure for a node in the linked list (to hold vehicles).
typedef struct Node {
    Vehicle* vehicle;    // Pointer to the vehicle.
    struct Node* next;   // Pointer to the next node in the list.
} Node;

Node* head = NULL;      // Pointer to the first node in the list.
Node* tail = NULL;      // Pointer to the last node in the list.

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;  // Mutex to ensure thread-safe operations.
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;     // Condition variable for thread synchronization.

char last_route = 'W';  // Direction of the last vehicle.
int same_route_count = 0;  // Counter for vehicles heading in the same direction.

struct timeval init_time;  // Timestamp marking the beginning of the program.
int next_vehicle_id = 0;   // ID for the next vehicle to be processed.

double elapsed_time();
void add_to_queue(Vehicle* v);
Vehicle* remove_from_queue();
int queue_length();
Vehicle* peek_queue();
int queue_length();
Vehicle* peek_queue();
int compare_vehicles(const void* a, const void* b);
void sort_queue();
void* vehicle_thread(void* arg);

int main(int argc, char* argv[]) {
    // Check if the correct number of command line arguments is provided.
    if (argc != 2) {
        // Print the correct usage of the program.
        printf("Usage: %s <input_file>\n", argv[0]);
        // Exit with status code 1 to indicate an error.
        exit(1);
    }

    // Get the current time and store it in init_time.
    gettimeofday(&init_time, NULL);

    // Open the input file for reading.
    FILE* file = fopen(argv[1], "r");
    // Check if the file was opened successfully.
    if (!file) {
        // Print an error message and exit with status code -1.
        perror("Error opening file");
        return -1;
    }

    int ch;
    int vehicle_num = 0;
    // Count the number of vehicles (lines) in the input file.
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            vehicle_num++;
        }
    }
    vehicle_num++;
    // Rewind the file pointer back to the beginning of the file.
    rewind(file);

    // Declare an array of Vehicle structures.
    Vehicle vehicles[vehicle_num];

    // Populate the vehicles array with data from the input file.
    for (int i = 0; i < vehicle_num; i++) {
        char dir;
        // Read the vehicle's direction, preparation duration, and travel duration from the file.
        fscanf(file, " %c %d %d", &dir, &vehicles[i].prep_duration, &vehicles[i].travel_duration);
        vehicles[i].vid = i;
        // Set the vehicle's route based on its direction.
        vehicles[i].route = dir == 'w' || dir == 'W' ? 'W' : 'E';
        // Set the vehicle's precedence based on its direction.
        vehicles[i].precedence = dir == 'w' || dir == 'e' ? 0 : 1;
    }
    // Close the input file.
    fclose(file);

    // Declare an array of thread identifiers.
    pthread_t threads[vehicle_num];
    // Create a thread for each vehicle.
    for (int i = 0; i < vehicle_num; i++) {
        pthread_create(&threads[i], NULL, vehicle_thread, &vehicles[i]);
    }

    // Wait for all threads to complete.
    for (int i = 0; i < vehicle_num; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

// Function to calculate the time elapsed since the beginning of the program.
double elapsed_time() {
    struct timeval curr;  
    gettimeofday(&curr, NULL);
    return (curr.tv_sec - init_time.tv_sec) + (curr.tv_usec - init_time.tv_usec) / 1000000.0;
}

// Function to add a vehicle to the linked list (queue).
void add_to_queue(Vehicle* v) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->vehicle = v;
    newNode->next = NULL;

    if (tail) {
        tail->next = newNode;
    } else {
        head = newNode;
    }
    tail = newNode;
}

// Function to remove and return the vehicle from the front of the list.
Vehicle* remove_from_queue() {
    if (!head) return NULL;

    Node* temp = head;
    Vehicle* v = temp->vehicle;
    head = head->next;

    if (!head) tail = NULL;

    free(temp);
    return v;
}

// Function to count the number of vehicles in the list.
int queue_length() {
    int len = 0;
    Node* temp = head;
    while (temp) {
        len++;
        temp = temp->next;
    }
    return len;
}

// Function to view (but not remove) the vehicle at the front of the list.
Vehicle* peek_queue() {
    if (head) {
        return head->vehicle;
    }
    return NULL;
}

// Comparison function to sort the vehicles based on their attributes.
int compare_vehicles(const void* a, const void* b) {
    Vehicle* vehA = *(Vehicle**)a;
    Vehicle* vehB = *(Vehicle**)b;

    if (vehA->precedence != vehB->precedence) {
        return vehB->precedence - vehA->precedence;
    }
    if (vehA->route == vehB->route) {
        if (vehA->set_time != vehB->set_time) {
            return vehA->set_time - vehB->set_time;
        }
        return vehA->vid - vehB->vid;
    }
    if (last_route == vehA->route) {
        return 1;
    }
    return -1;
}

// Function to sort the vehicles in the list based on the comparison function.
void sort_queue() {
    int len = queue_length();
    if (len <= 0) return;

    Vehicle* arr[len];
    Node* temp = head;
    for (int i = 0; i < len; i++) {
        arr[i] = temp->vehicle;
        temp = temp->next;
    }

    qsort(arr, len, sizeof(Vehicle*), compare_vehicles);

    temp = head;
    for (int i = 0; i < len; i++) {
        temp->vehicle = arr[i];
        temp = temp->next;
    }
}

// Thread function to handle vehicle operations.
void* vehicle_thread(void* arg) {
    Vehicle* v = (Vehicle*)arg;

    // Wait for the vehicle's turn based on its ID.
    while(1) {
        pthread_mutex_lock(&mtx);
        if (v->vid == next_vehicle_id) {
            next_vehicle_id++;
            pthread_mutex_unlock(&mtx);
            break;
        }
        pthread_mutex_unlock(&mtx);
        usleep(1000);
    }

    // Simulate the preparation duration.
    usleep(v->prep_duration * 100000);
    printf("00:00:0%.1f Vehicle %d is ready to go %s\n", elapsed_time(), v->vid, (v->route == 'E') ? "East" : "West");

    pthread_mutex_lock(&mtx);
    v->set_time = v->prep_duration;
    add_to_queue(v);
    sort_queue();

    // Logic to handle vehicle travel and ensure synchronization.
    while (1) {
        Vehicle* next_veh = peek_queue();

        if (same_route_count >= 3) {
            if (next_veh->route != last_route) {
                break;
            }
            remove_from_queue();
            next_veh = peek_queue();
        }

        if (next_veh == v) {
            same_route_count = (last_route == v->route) ? (same_route_count + 1) : 1;
            last_route = v->route;
            remove_from_queue();
            break;
        } else {
            pthread_cond_wait(&cv, &mtx);
        }
    }

    printf("00:00:0%.1f Vehicle %d is ON the main route going %s\n", elapsed_time(), v->vid, (v->route == 'E') ? "East" : "West");
    usleep(v->travel_duration * 100000);
    printf("00:00:0%.1f Vehicle %d is OFF the main route after going %s\n", elapsed_time(), v->vid, (v->route == 'E') ? "East" : "West");

    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&mtx);

    return NULL;
}