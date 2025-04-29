#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cmath>
#include <cstring>

using namespace std;

// BONUS PROJECT: Parallel Array Summation Using a Process Tree with Pipes
// -----------------------------------------------------------------------
// This project constructs a complete binary tree of processes. 
// The tree has a specified height H and 2^(H-1) leaves.
// The input array is divided into equal-sized chunks (with padding);
// each leaf computes the sum of its chunk and sends it to its parent via a pipe.
// Each internal node waits for its two child sums via pipes, adds them, 
// and sends the result upward. Finally, the root process computes the final sum 
// and prints it along with detailed messages showing process IDs, 
// node indexes, levels, and positions.
// -----------------------------------------------------------------------

struct NodeInfo {
    int index, level, position;
    vector<int> values;
};

// Function to compute the sum at a leaf node
void computeSum(NodeInfo node, int write_fd, int term_fd) {
    // Step: Compute the sum of the assigned chunk of the array.
    int sum = 0;
    for (int v : node.values) sum += v;
    // Step: Print detailed message for this leaf.
    cout << "[PID " << getpid() << "] [Index " << node.index << "] [Level " << node.level
         << ", Position " << node.position << "] computed sum: " << sum << endl;
    // Step: Send the sum to the parent process.
    write(write_fd, &sum, sizeof(sum));
    close(write_fd);
    // Wait for termination signal (EOF from term_fd) before terminating.
    char dummy;
    read(term_fd, &dummy, 1);
    cout << "[PID " << getpid() << "] [Index " << node.index << "] terminated." << endl;
    exit(0);
}

// The recursive function to build the process tree and sum array parts
void internalNode(NodeInfo node, int write_fd, int term_fd, int height, vector<vector<int> >& chunks) {
    // Step: If this is a leaf node (level equals height), compute its sum.
    if (node.level == height) {
        computeSum(node, write_fd, term_fd);
        // ...unreachable...
    }

    int left_pipe[2], right_pipe[2];
    pipe(left_pipe); 
    pipe(right_pipe);
    
    // Fork left child process.
    pid_t left = fork();
    if (left == 0) {
        // Compute left child's index using binary tree mapping.
        NodeInfo left_child = { node.index * 2 + 1, node.level + 1, node.position * 2, vector<int>() };
        if (node.level + 1 == height)  // child is a leaf
            left_child.values = chunks[left_child.position];
        internalNode(left_child, left_pipe[1], term_fd, height, chunks);
    }

    // Fork right child process.
    pid_t right = fork();
    if (right == 0) {
        // Compute right child's index using binary tree mapping.
        NodeInfo right_child = { node.index * 2 + 2, node.level + 1, node.position * 2 + 1, vector<int>() };
        if (node.level + 1 == height)  // child is a leaf
            right_child.values = chunks[right_child.position];
        internalNode(right_child, right_pipe[1], term_fd, height, chunks);
    }

    close(left_pipe[1]); 
    close(right_pipe[1]);
    int sum_left, sum_right;
    read(left_pipe[0], &sum_left, sizeof(sum_left));
    read(right_pipe[0], &sum_right, sizeof(sum_right));
    int total = sum_left + sum_right;
    
    if (node.index == 0)
        cout << "[PID " << getpid() << "] [Index " << node.index << "] [Level " << node.level 
             << ", Position " << node.position
             << "] recieved: " << sum_left << ", " << sum_right << " -> Final sum: " << total << endl;
    else
        cout << "[PID " << getpid() << "] [Index " << node.index << "] [Level " << node.level 
             << ", Position " << node.position
             << "] received: " << sum_left << ", " << sum_right << " -> sum: " << total << endl;

    write(write_fd, &total, sizeof(total));
    close(write_fd);

    waitpid(left, NULL, 0);
    waitpid(right, NULL, 0);

    // Wait for termination signal from term_fd before terminating.
    char dummy;
    read(term_fd, &dummy, 1);
    cout << "[PID " << getpid() << "] [Index " << node.index << "] terminated." << endl;
    exit(0);
}

int main() {
    // Step: Read the height (H) of the process tree and the number (M) of integers.
    int H, M;
    cin >> H >> M;
    
    // Step: Read the input array of integers.
    vector<int> input(M);
    for (int& val : input) 
        cin >> val;

    // Step: Calculate the number of leaves in the binary tree.
    int num_leaves = pow(2, H - 1);
    
    // Step: Pad the array so that its size is a multiple of number of leaves.
    int padded_size = ((M + num_leaves - 1) / num_leaves) * num_leaves;
    while (input.size() < padded_size) 
        input.push_back(0);

    // Step: Divide the array into equal chunks (one per leaf process).
    int chunk_size = padded_size / num_leaves;
    vector<vector<int> > chunks(num_leaves);
    for (int i = 0; i < num_leaves; ++i)
        chunks[i] = vector<int>(input.begin() + i * chunk_size, input.begin() + (i + 1) * chunk_size);

    // Step: Create the pipe between the root and its child processes.
    int pipe_fd[2];
    pipe(pipe_fd);
    // Create a termination pipe to signal processes to exit later.
    int term_pipe[2];
    pipe(term_pipe);

    // Step: Fork the root process that launches the recursion.
    pid_t pid = fork();
    if (pid == 0) {
        close(term_pipe[1]); // Ensure child does not hold the termination write end.
        // The root node now always has index 0.
        NodeInfo root = {0, 1, 0, vector<int>()};
        internalNode(root, pipe_fd[1], term_pipe[0], H, chunks);
    }
    close(pipe_fd[1]);
    
    // Step: Root process reads the final sum from its child.
    int final_sum;
    read(pipe_fd[0], &final_sum, sizeof(final_sum));
    
    // Signal termination to all child processes before waiting for them.
    close(term_pipe[1]);
    
    // Ensure all child processes terminate before exiting.
    pid_t wpid;
    while ((wpid = wait(NULL)) > 0) {
        cout << "[PID " << wpid << "] terminated." << endl;
    }
    return 0;
}