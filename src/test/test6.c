main()
{
    int a, s;
    a = 1;
    s = 0;
    while (a <= 10) {
        s = s + a;
        a = a+1;
    }
    put_int(a);
    put_int(s);
}
