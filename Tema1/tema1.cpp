#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <pthread.h>
#include <set>
#include <string>
#include <vector>

using namespace std;

struct arg {
  int R;
  int M;
  int id;
  pthread_mutex_t *lock;
  vector<string> *files;
  pthread_barrier_t *barrier;
  vector<vector<long>> *result;
};

/* functie care intoarce un vector de numere citite dintr-un fisier */
vector<long> getNoVecFromFile(string filename) {
  vector<long> numbers;
  int fileNo;
  string line;
  ifstream in(filename);
  getline(in, line);
  fileNo = atoi(line.c_str());

  while (fileNo) {
    getline(in, line);
    numbers.push_back(atoi(line.c_str()));
    fileNo--;
  }

  return numbers;
}

/* verifica daca un numar este putere perfecta pt un exponent dat */
bool check_power_binary_search(int low, long high, int exp, long num) {
  if (num == 1)
    return true;
  long mid;
  while (low <= high) {
    mid = (low + high) / 2;
    /* succes */
    if (pow(mid, exp) == num)
      return true;
    /* 
     *daca rezultatul ridicarii e prea mare trebuie 
     * sa merge spre partea inferioara 
     */
    else if (pow(mid, exp) > num)
      high = mid - 1;
    /* spre partea superioara daca e mai mic */
    else
      low = mid + 1;
  }
  return false;
}

void *fmap(void *arg) {
  struct arg *aux = (struct arg *)arg;
  vector<long> numbers;
  string curr_file = "not ready";
  while (aux->files->size() > 0) {
    /* 
     *vreau ca un singur thread sa stearga fisierul 
     *prelucrat la un moment dat asa ca fol mutex 
     */
    pthread_mutex_lock(aux->lock);
    if (aux->files->size() > 0) {
      curr_file = aux->files->at(0);
      aux->files->erase(aux->files->begin());
    }
    pthread_mutex_unlock(aux->lock);
    /* daca am un fisier de prelucrat il prelucrez */
    if (curr_file != "not ready") {
      numbers = getNoVecFromFile(curr_file);
      for (int i = 2; i <= aux->R + 1; ++i) {
        for (int j = 0; j < (int)numbers.size(); ++j) {
          /* 
           *pt toate numerele din fisier verific daca sunt 
           *putere perfecta pt vreun exponent [din 2,R] 
          */
          if (check_power_binary_search(2, (long)(sqrt(numbers[j]) + 1), i, numbers[j])) {
            /* vreau ca un singur thread sa scrie in rezultat la un mom dat */
            pthread_mutex_lock(aux->lock);
            aux->result->at(i - 2).push_back(numbers[j]);
            pthread_mutex_unlock(aux->lock);
          }
        }
      }
    }
  }
  /* aici vor ajunge M mapperi, bariera va mai astepta inca R thread-uri */
  pthread_barrier_wait(aux->barrier);
  pthread_exit(NULL);
}

void *freduce(void *arg) {
  struct arg *aux = (struct arg *)arg;
  /* aici ajung restul de R reduceri */
  pthread_barrier_wait(aux->barrier);
  ofstream out("out" + to_string(aux->id) + ".txt");
  /* se elimina duplicatele din lista cu index = id_thread */
  set<long> s(aux->result->at(aux->id - 2).begin(),
              aux->result->at(aux->id - 2).end());
  /* se scrie in fisierul marimea listei fara duplicate */
  out << (int)s.size();
  out.close();
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int fileNo;
  void *status;
  int M = atoi(argv[1]);
  int R = atoi(argv[2]);

  /* structurile de date 'globale' din structura */
  vector<string> *files = new vector<string>();
  vector<vector<long>> *result = new vector<vector<long>>(R);
  result->reserve(R);
  pthread_barrier_t *barrier = new pthread_barrier_t();
  pthread_mutex_t *mutex = new pthread_mutex_t();

  if (pthread_barrier_init(barrier, NULL, M + R) != 0) {
    printf("\n mutex init failed\n");
    return 1;
  }
  if (pthread_mutex_init(mutex, NULL) != 0) {
    printf("\n mutex init failed\n");
    return 1;
  }

  pthread_t mThreads[M];
  pthread_t rThreads[R];
  vector<struct arg> args(M + R);
  string filename = argv[3];
  ifstream in(filename);
  string line;
  getline(in, line);
  fileNo = atoi(line.c_str());

  /* pun numele fisierelor in lista "files" din cel dat ca argument */
  while (fileNo) {
    getline(in, line);
    if (line == "\n")
      continue;
    files->push_back(line);
    fileNo--;
  }

  for (int i = 0; i < M; i++) {
    /* completez instanta fiecarui thread mapper */
    args[i].R = R;
    args[i].M = M;
    args[i].id = 0;
    args[i].barrier = barrier;
    args[i].lock = mutex;
    args[i].files = files;
    args[i].result = result;
    int r = pthread_create(&mThreads[i], NULL, fmap, (void *)&args[i]);
    if (r) {
      printf("Eroare la crearea thread-ului %d\n", i);
      exit(-1);
    }
  }

  for (int i = 0; i < R; i++) {
    /* completez instanta fiecarui thread reducer */
    args[i + M].id = i + 2;
    args[i + M].result = result;
    args[i + M].barrier = barrier;
    int r = pthread_create(&rThreads[i], NULL, freduce, (void *)&args[i + M]);
    if (r) {
      printf("Eroare la crearea thread-ului %d\n", i);
      exit(-1);
    }
  }

  for (int i = 0; i < M; i++) {
    int r = pthread_join(mThreads[i], &status);
    if (r) {
      printf("Eroare la asteptarea thread-ului %d\n", i);
      exit(-1);
    }
  }

  for (int i = 0; i < R; i++) {
    int r = pthread_join(rThreads[i], &status);
    if (r) {
      printf("Eroare la asteptarea thread-ului %d\n", i);
      exit(-1);
    }
  }
  pthread_barrier_destroy(barrier);

  /* eliberez memoria */
  in.close();
  files->clear();
  files->shrink_to_fit();
  result->clear();
  result->shrink_to_fit();
  delete files;
  delete result;
  delete barrier;
  delete mutex;
  return 0;
}