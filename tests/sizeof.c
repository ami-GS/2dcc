int main() {
    int a[3] = {1,2,3};
    return sizeof(a) / sizeof(a[0]) + sizeof a / sizeof a[0] + 94;
}