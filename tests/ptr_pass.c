int pppp(int *array, int C) {
    int ppppArray[5] = {11,22,33,44,55};
    return array[2] + 2*ppppArray[3] + C;
}
int main() {
    int array[3] = {1,2,3};
    return pppp(array, 9);
}