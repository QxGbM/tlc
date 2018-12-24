main()
{
    int a, b;

    a = 2; b = 1;
    if (a > b) {
        a = b+1;
    }

    if (b == a) {
        a = a + b;
    } else {
        a = a - b;
    }
    put_int(a);
}
