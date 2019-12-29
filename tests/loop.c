int main() {
    int c = 25;
    int ret = 0;
    int i = 0;
    while (i < 4) {
        ret = ret + c;
        i = i + 1;
    }

    for (int i = 0; i < 4; i = i + 1) {
        ret = ret + c;
    }
    return ret/2;
}