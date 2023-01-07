#include "mpi.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>

using namespace std;

void print_vec(vector<int> vec) {
  cout << "The VECTORRRRRRRRRRRRRRRR = {";
  for (int i = 0; i < (int)vec.size(); i++) {
    if (i == (int)vec.size() - 1) {
      cout << vec[i] << "}" << endl;
    }
    cout << vec[i] << " ";
  }
}

inline int min(int a, int b) {
  if (a > b)
    return b;
  return a;
}

pair<string, MPI_Status> receive_str(string topology, int source, int tag) {
  MPI_Status status;
  int count;
  MPI_Probe(source, tag, MPI_COMM_WORLD, &status);
  MPI_Get_count(&status, MPI_CHAR, &count);

  // Allocate a buffer and receive the message
  char *buffer = new char[count];
  MPI_Recv(buffer, count, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);

  // Construct a string from the received message
  string received_str(buffer, count);
  delete[] buffer;
  string forward_str = received_str + " " + topology;
  return make_pair(forward_str, status);
}

int main(int argc, char *argv[]) {
  int numtasks, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int vector_size = atoi(argv[1]);
  bool round_up = true;

  // First process starts the circle.
  if (rank == 0) {
    int N;
    vector<int> zeros_workers;
    vector<int> numbers;
    string zeros_topology = "0:";
    ifstream in("cluster0.txt");
    in >> N;
    for (int i = 0; i < N; i++) {
      int x;
      in >> x;
      if (i == N - 1) {
        zeros_topology += to_string(x);
      } else {
        zeros_topology += to_string(x) + ",";
      }
      zeros_workers.push_back(x);
    }
    for (int i = 0; i < vector_size; i++) {
      numbers.push_back(vector_size - i - 1);
    }
    vector<int> zeros_partitions;
    vector<int> ones_partitions;
    vector<int> twos_partitions;
    vector<int> threes_partitions;
    int zeros_part = (double)vector_size / (numtasks - 4);
    // cout << "vector_size " << zeros_part << endl;
    if (vector_size % numtasks == 0 && round_up) {
      zeros_part++;
      round_up = false;
    } else {
      round_up = true;
    }
    MPI_Status status;

    string topology = zeros_topology + " " + receive_str("", 3, 3).first;
    cout << "0 -> " << topology << endl;
    MPI_Send(topology.c_str(), topology.size(), MPI_CHAR, 3, 0, MPI_COMM_WORLD);
    cout << "M(0,3)" << endl;
    for (int i = 0; i < N; i++) {
      MPI_Send(topology.c_str(), topology.size(), MPI_CHAR, zeros_workers[i], 3,
               MPI_COMM_WORLD);
      cout << "M(0," << zeros_workers[i] << ")" << endl;
    }
    int ID = 0;
    for (int i = 0; i < N; i++) {
      int start = ID * (double)vector_size / (numtasks - 4);
      int end =
          min((ID + 1) * (double)vector_size / (numtasks - 4), vector_size);
      int vec_size = end - start;
      MPI_Send(&vec_size, 1, MPI_INT, zeros_workers[i], 0, MPI_COMM_WORLD);
      vector<int> tosend;
      tosend.assign(numbers.begin() + start, numbers.begin() + end);
      ID++;
      //   cout << "size=" << tosend[1] <<endl;
      // Send the data in the vector to the destination rank
      MPI_Send(tosend.data(), vec_size, MPI_INT, zeros_workers[i], 0,
               MPI_COMM_WORLD);
    }
    MPI_Send(&ID, 1, MPI_INT, 3, 0, MPI_COMM_WORLD);
    MPI_Send(numbers.data(), vector_size, MPI_INT, 3, 0, MPI_COMM_WORLD);
    vector<int> final_vec;
    for (int i = 0; i < N; i++) {
      int size;
      MPI_Recv(&size, 1, MPI_INT, zeros_workers[i], zeros_workers[i],
               MPI_COMM_WORLD, &status);
      vector<int> mul_vec(size);
      MPI_Recv(mul_vec.data(), size, MPI_INT, zeros_workers[i],
               zeros_workers[i], MPI_COMM_WORLD, &status);
        for (int elem : mul_vec) {
          final_vec.push_back(elem);
        }
    }
    int threes_size;
    MPI_Recv(&threes_size, 1, MPI_INT, 3, 3, MPI_COMM_WORLD, &status);
    vector<int> threes_vec(threes_size);
    MPI_Recv(threes_vec.data(), threes_size, MPI_INT, 3, 3, MPI_COMM_WORLD,
             &status);
    for (int elem : threes_vec) {
      final_vec.push_back(elem);
    }
    cout << "Rezultat: ";
    for (int i = 0; i < (int)final_vec.size(); i++) {
      if (i == (int)final_vec.size() - 1) {
        cout << final_vec[i] << endl;
      }
      cout << final_vec[i] << " ";
    }

  } else if (rank == 1) {
    int N;
    vector<int> ones_workers;
    string ones_topology = "1:";
    ifstream in("cluster1.txt");
    in >> N;
    for (int i = 0; i < N; i++) {
      int x;
      in >> x;
      if (i == N - 1) {
        ones_topology += to_string(x);
      } else {
        ones_topology += to_string(x) + ",";
      }
      ones_workers.push_back(x);
    }

    MPI_Status status;

    MPI_Send(ones_topology.c_str(), ones_topology.size(), MPI_CHAR, 2, 1,
             MPI_COMM_WORLD);
    cout << "M(1,2)" << endl;
    string topology = receive_str("", 2, 2).first;
    cout << "1 -> " << topology << endl;
    for (int i = 0; i < N; i++) {
      MPI_Send(topology.c_str(), topology.size(), MPI_CHAR, ones_workers[i], 3,
               MPI_COMM_WORLD);
      cout << "M(1," << ones_workers[i] << ")" << endl;
    }
    int ID;
    vector<int> numbers(vector_size);
    MPI_Recv(&ID, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, &status);
    MPI_Recv(numbers.data(), vector_size, MPI_INT, 2, 2, MPI_COMM_WORLD,
             &status);
    for (int i = 0; i < N; i++) {
      int start = ID * (double)vector_size / (numtasks - 4);
      int end =
          min((ID + 1) * (double)vector_size / (numtasks - 4), vector_size);
      int vec_size = end - start;
      MPI_Send(&vec_size, 1, MPI_INT, ones_workers[i], 1, MPI_COMM_WORLD);
      vector<int> tosend;
      tosend.assign(numbers.begin() + start, numbers.begin() + end);
      ID++;
      //   cout << "size=" << tosend[1] <<endl;
      // Send the data in the vector to the destination rank
      MPI_Send(tosend.data(), vec_size, MPI_INT, ones_workers[i], 1,
               MPI_COMM_WORLD);
    }
    vector<int> final_vec;
    for (int i = 0; i < N; i++) {
      int size;
      MPI_Recv(&size, 1, MPI_INT, ones_workers[i], ones_workers[i],
               MPI_COMM_WORLD, &status);
      vector<int> mul_vec(size);
      MPI_Recv(mul_vec.data(), size, MPI_INT, ones_workers[i],
      ones_workers[i],
               MPI_COMM_WORLD, &status);
      for (int elem : mul_vec) {
        final_vec.push_back(elem);
      }
    }
    int final_size = final_vec.size();
    MPI_Send(&final_size, 1, MPI_INT, 2, 1, MPI_COMM_WORLD);
    MPI_Send(final_vec.data(), final_size, MPI_INT, 2, 1, MPI_COMM_WORLD);

  } else if (rank == 2) {
    int N;
    vector<int> twos_workers;
    string twos_topology = "2:";
    ifstream in("cluster2.txt");
    in >> N;
    for (int i = 0; i < N; i++) {
      int x;
      in >> x;
      if (i == N - 1) {
        twos_topology += to_string(x);
      } else {
        twos_topology += to_string(x) + ",";
      }
      twos_workers.push_back(x);
    }

    MPI_Status status;
    string forward_str = receive_str(twos_topology, 1, 1).first;
    MPI_Send(forward_str.c_str(), forward_str.size(), MPI_CHAR, 3, 2,
             MPI_COMM_WORLD);
    cout << "M(2,3)" << endl;
    string topology = receive_str("", 3, 3).first;
    cout << "2 -> " << topology << endl;
    MPI_Send(topology.c_str(), topology.size(), MPI_CHAR, 1, 2, MPI_COMM_WORLD);
    cout << "M(2,1)" << endl;
    for (int i = 0; i < N; i++) {
      MPI_Send(topology.c_str(), topology.size(), MPI_CHAR, twos_workers[i], 3,
               MPI_COMM_WORLD);
      cout << "M(2," << twos_workers[i] << ")" << endl;
    }
    int ID;
    vector<int> numbers(vector_size);
    MPI_Recv(&ID, 1, MPI_INT, 3, 3, MPI_COMM_WORLD, &status);
    MPI_Recv(numbers.data(), vector_size, MPI_INT, 3, 3, MPI_COMM_WORLD,
             &status);
    for (int i = 0; i < N; i++) {
      int start = ID * (double)vector_size / (numtasks - 4);
      int end =
          min((ID + 1) * (double)vector_size / (numtasks - 4), vector_size);
      int vec_size = end - start;
      MPI_Send(&vec_size, 1, MPI_INT, twos_workers[i], 2, MPI_COMM_WORLD);
      vector<int> tosend;
      tosend.assign(numbers.begin() + start, numbers.begin() + end);
      ID++;
      //   cout << "size=" << tosend[1] <<endl;
      // Send the data in the vector to the destination rank
      MPI_Send(tosend.data(), vec_size, MPI_INT, twos_workers[i], 2,
               MPI_COMM_WORLD);
    }
    MPI_Send(&ID, 1, MPI_INT, 1, 2, MPI_COMM_WORLD);
    MPI_Send(numbers.data(), vector_size, MPI_INT, 1, 2, MPI_COMM_WORLD);
    vector<int> final_vec;
    for (int i = 0; i < N; i++) {
      int size;
      MPI_Recv(&size, 1, MPI_INT, twos_workers[i], twos_workers[i],
               MPI_COMM_WORLD, &status);
      vector<int> mul_vec(size);
      MPI_Recv(mul_vec.data(), size, MPI_INT, twos_workers[i],
      twos_workers[i],
               MPI_COMM_WORLD, &status);
      for (int elem : mul_vec) {
        final_vec.push_back(elem);
      }
    }
    int ones_size;
    MPI_Recv(&ones_size, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &status);
    vector<int> ones_vec(ones_size);
    MPI_Recv(ones_vec.data(), ones_size, MPI_INT, 1, 1, MPI_COMM_WORLD,
             &status);
    for (int elem : ones_vec) {
      final_vec.push_back(elem);
    }
    int final_size = final_vec.size();
    MPI_Send(&final_size, 1, MPI_INT, 3, 2, MPI_COMM_WORLD);
    MPI_Send(final_vec.data(), final_size, MPI_INT, 3, 2, MPI_COMM_WORLD);
  } else if (rank == 3) {
    int N;
    vector<int> threes_workers;
    string threes_topology = "3:";
    ifstream in("cluster3.txt");
    in >> N;
    for (int i = 0; i < N; i++) {
      int x;
      in >> x;
      if (i == N - 1) {
        threes_topology += to_string(x);
      } else {
        threes_topology += to_string(x) + ",";
      }
      threes_workers.push_back(x);
    }

    MPI_Status status;
    string forward_str = receive_str(threes_topology, 2, 2).first;
    MPI_Send(forward_str.c_str(), forward_str.size(), MPI_CHAR, 0, 3,
             MPI_COMM_WORLD);
    cout << "M(3,0)" << endl;
    string topology = receive_str(threes_topology, 0, 0).first;
    cout << "3 -> " << topology << endl;
    MPI_Send(topology.c_str(), topology.size(), MPI_CHAR, 2, 3, MPI_COMM_WORLD);
    cout << "M(3,2)" << endl;
    for (int i = 0; i < N; i++) {
      MPI_Send(topology.c_str(), topology.size(), MPI_CHAR, threes_workers[i],
               3, MPI_COMM_WORLD);
      cout << "M(3," << threes_workers[i] << ")" << endl;
    }
    int ID;
    vector<int> numbers(vector_size);
    MPI_Recv(&ID, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv(numbers.data(), vector_size, MPI_INT, 0, 0, MPI_COMM_WORLD,
             &status);
    for (int i = 0; i < N; i++) {
      int start = ID * (double)vector_size / (numtasks - 4);
      int end =
          min((ID + 1) * (double)vector_size / (numtasks - 4), vector_size);
      int vec_size = end - start;
      MPI_Send(&vec_size, 1, MPI_INT, threes_workers[i], 3, MPI_COMM_WORLD);
      vector<int> tosend;
      tosend.assign(numbers.begin() + start, numbers.begin() + end);
      ID++;
      //   cout << "size=" << tosend[1] <<endl;
      // Send the data in the vector to the destination rank
      MPI_Send(tosend.data(), vec_size, MPI_INT, threes_workers[i], 3,
               MPI_COMM_WORLD);
    }
    MPI_Send(&ID, 1, MPI_INT, 2, 3, MPI_COMM_WORLD);
    MPI_Send(numbers.data(), vector_size, MPI_INT, 2, 3, MPI_COMM_WORLD);
    vector<int> final_vec;
    for (int i = 0; i < N; i++) {
      int size;
      MPI_Recv(&size, 1, MPI_INT, threes_workers[i], threes_workers[i],
               MPI_COMM_WORLD, &status);
      vector<int> mul_vec(size);
      MPI_Recv(mul_vec.data(), size, MPI_INT, threes_workers[i],
               threes_workers[i], MPI_COMM_WORLD, &status);
      for (int elem : mul_vec) {
        final_vec.push_back(elem);
      }
    }
    int twos_size;
    MPI_Recv(&twos_size, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, &status);
    vector<int> twos_vec(twos_size);
    MPI_Recv(twos_vec.data(), twos_size, MPI_INT, 2, 2, MPI_COMM_WORLD,
             &status);
    for (int elem : twos_vec) {
      final_vec.push_back(elem);
    }
    int final_size = final_vec.size();
    MPI_Send(&final_size, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
    MPI_Send(final_vec.data(), final_size, MPI_INT, 0, 3, MPI_COMM_WORLD);

  } else {
    pair<string, MPI_Status> response =
        receive_str("", MPI_ANY_SOURCE, MPI_ANY_TAG);
    string topology = response.first;
    MPI_Status status = response.second;
    cout << rank << " -> " << topology << endl;
    int size;
    MPI_Recv(&size, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
             &status);
    vector<int> numbers(size);
    MPI_Recv(numbers.data(), size, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
             MPI_COMM_WORLD, &status);
    int multiplier = 5;
    transform(numbers.begin(), numbers.end(), numbers.begin(),
              [multiplier](int x) { return x * multiplier; });

    // cout << "SIZEEEE" << size << endl;

    MPI_Send(&size, 1, MPI_INT, status.MPI_SOURCE, rank, MPI_COMM_WORLD);
    MPI_Send(numbers.data(), size, MPI_INT, status.MPI_SOURCE, rank,
             MPI_COMM_WORLD);
  }

  MPI_Finalize();
}
