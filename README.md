Compile and Run C Program:

1. Normal C Program
   gcc program.c -o program.exe
   ./program.exe
   
3. GTK C Program — Without SQLite
   gcc program.c -o program.exe $(pkg-config --cflags --libs gtk+-3.0)
  ./program.exe
   
4. GTK C Program with SQLite
   gcc program.c -o program.exe $(pkg-config --cflags --libs gtk+-3.0) -lsqlite3
   ./program.exe
