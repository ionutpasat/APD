#include <iostream>
#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace std;

int N;
int P;
vector<int> array;
vector<int> prefixSum;
int sum = 0;
pthread_barrier_t barrier;
pthread_mutex_t mutex;


// functie de afisare vector
void print_vec(vector<int> in) {
  cout << "vector is { ";
  for (int e : in) {
    cout << e << " ";
  }
  cout << "}" << endl;
}

// functia argument a threadurilor
void *threadFunction(void *arg) {
    int threadId = *(int*)arg;

    int start = (N/P) * threadId;
    int end = (N/P) * (threadId + 1);

//suma de prefixe
    if (threadId == 0) {
        prefixSum[start] = array[start];
        for (int i = start + 1; i < end; i++) {
            pthread_mutex_lock(&mutex);
            prefixSum[i] = prefixSum[i-1] + array[i];
            pthread_mutex_unlock(&mutex);
        }
    } else {
        prefixSum[start] = prefixSum[start - 1] + array[start];
        for (int i = start + 1; i < end; i++) {
            pthread_mutex_lock(&mutex);
            prefixSum[i] = prefixSum[i-1] + array[i];
            pthread_mutex_unlock(&mutex);
        }
    }

//se asteapta ca toate threadurile sa si termine treaba
    pthread_barrier_wait(&barrier);

//odata terminata suma este afisata de threadul 0
    if (threadId == 0) {
        printf("Sum: %d\n", prefixSum[N-1]);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  N = atoi(argv[1]);
  P = atoi(argv[2]);
  pthread_t threads[P];
  int threadIds[P];

  // se rezerva elementele pentru array si prefixSum
  array.reserve(N);
  prefixSum.reserve(N);

  // se adauga elementele primite in commandline
  for (int i = 3; i < N + 3; ++i) {
    array.push_back(atoi(argv[i]));
  }

  print_vec(array);

  // se initializeaza bariera
  pthread_barrier_init(&barrier, NULL, P);
  pthread_mutex_init(&mutex, NULL);

  for (int i = 0; i < P; i++) {
    threadIds[i] = i;
    pthread_create(&threads[i], NULL, threadFunction, &threadIds[i]);
  }

  for (int i = 0; i < P; i++) {
    pthread_join(threads[i], NULL);
  }

    //se distrug lockurile
  pthread_barrier_destroy(&barrier);
  pthread_mutex_destroy(&mutex);

  return 0;
}
