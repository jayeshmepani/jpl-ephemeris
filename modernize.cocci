@@
expression p, n;
@@
- bzero(p, n)
+ memset(p, 0, n)

@@
expression s, d, n;
@@
- bcopy(s, d, n)
+ memmove(d, s, n)

@@
expression s, c;
@@
- index(s, c)
+ strchr(s, c)

@@
expression s, c;
@@
- rindex(s, c)
+ strrchr(s, c)
