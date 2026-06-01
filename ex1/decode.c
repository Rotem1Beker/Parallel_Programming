// 217386598 Rotem Beker

long decode_c_version(long x, long y, long z) {
    y = y - z;
    x = x * y;
    long num = y;
    num = num << 63;
    num = num >> 63;
    return (num ^ x);
}