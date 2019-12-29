int main() {
    int ret = 1;
    if (ret < 2)
        ret = ret * 2;
    if (ret <= 2)
        ret = 3 * ret;
    if (ret == 6)
        ret = ret * 5;
    if (ret != 29)
        ret = 7 * ret;
    if (ret > 209)
        ret = ret + 11;
    if (ret >= 221)
        ret = 13 + ret;
    return ret - 134;
}